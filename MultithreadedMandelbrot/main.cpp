#include <vector>
#include <complex>
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>
#include <webgpu/webgpu.h>
#include <cassert>
#include <vector>
#include "WebgpuUtilities.h"

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>


#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

#define Multi 1;

constexpr int WIDTH = 1920, HEIGHT = 1920, MAX_IT = 10500, THR_COUNT =128;

constexpr double zoom =1,center_x =0.0, center_y=0.0;



constexpr int LINE = HEIGHT / THR_COUNT;

struct RGB { int r, g, b; };

//RGB hsvToRgb(float h, float s, float v) {
//    float c = v * s;
//    float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
//    float m = v - c;
//    float rp, gp, bp;
//
//    if (h < 60) { rp = c; gp = x; bp = 0; }
//    else if (h < 120) { rp = x; gp = c; bp = 0; }
//    else if (h < 180) { rp = 0; gp = c; bp = x; }
//    else if (h < 240) { rp = 0; gp = x; bp = c; }
//    else if (h < 300) { rp = x; gp = 0; bp = c; }
//    else { rp = c; gp = 0; bp = x; }
//
//    return {
//        int((rp + m) * 255),
//        int((gp + m) * 255),
//        int((bp + m) * 255)
//    };
//}


double mandelbrot(double cr, double ci)
{

    std::complex<double> z = 0;
    std::complex<double> c(cr, ci);

    int i = 0;

    while (std::abs(z) <= 2.0 && i < MAX_IT) 
    {

        z = z * z + c;
        
        ++i;

    }
  
    if (i == MAX_IT) 
    {
        return float(MAX_IT);
    }

    double nu = i + 1 - std::log(std::log(std::abs(z))) / std::log(2.0);
    return double(nu);


}


void renderRow(int thr_k, std::vector<double>& buf)
{
 
    double scaleX = 4.0 / (WIDTH * zoom);
    double scaleY = 4.0 / (HEIGHT * zoom);

    for (int y = thr_k * LINE; y < std::min(thr_k * LINE + LINE, HEIGHT); ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double cr = center_x + (x - WIDTH / 2.0) * scaleX;
            double ci = center_y + (y - HEIGHT / 2.0) * scaleY;
            buf[y * WIDTH + x] = mandelbrot(cr, ci);
        }
    }
}



class MandelbrotRenderer 
{
public:
	
	bool Init();

	void Terminate();

	void Tick();

	bool IsRunning();

private:
	
	GLFWwindow* window;
	WGPUDevice device;
	WGPUQueue queue;
	WGPUSurface surface;
};

int main() {
	MandelbrotRenderer app;

	if (!app.Init()) {
		return 1;
	}

#ifdef __EMSCRIPTEN__
	
	auto callback = [](void* arg) {
		MandelbrotRenderer* pApp = reinterpret_cast<MandelbrotRenderer*>(arg);
		pApp->Tick();
		};
	emscripten_set_main_loop_arg(callback, &app, 0, true);
	//                                    
#else // __EMSCRIPTEN__
	while (app.IsRunning()) {
		app.Tick();
	}
#endif // __EMSCRIPTEN__

	app.Terminate();

	return 0;
}

bool MandelbrotRenderer::Init() 
{
	
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); 
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(640, 480, "Learn WebGPU", nullptr, nullptr);

	// Create instance
	WGPUInstance instance = wgpuCreateInstance(nullptr);

	// Get adapter
	std::cout << "Requesting adapter..." << std::endl;
	surface = glfwGetWGPUSurface(instance, window);

	WGPURequestAdapterOptions adapterOpts = {};
	adapterOpts.nextInChain = nullptr;
	adapterOpts.compatibleSurface = surface;

	WGPUAdapter adapter = requestAdapterSync(instance, &adapterOpts);
	std::cout << "Got adapter: " << adapter << std::endl;

	
	wgpuInstanceRelease(instance);

	// Get device
	std::cout << "Requesting device..." << std::endl;
	WGPUDeviceDescriptor deviceDesc = {};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "My Device";
	deviceDesc.requiredFeatureCount = 0; 
	deviceDesc.requiredLimits = nullptr;
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";
	deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
		std::cout << "Device lost: reason " << reason;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
};
	device = requestDeviceSync(adapter, &deviceDesc);
	std::cout << "Got device: " << device << std::endl;

	
	wgpuAdapterRelease(adapter);

	
	auto onDeviceError = [](WGPUErrorType type, char const* message, void* /* pUserData */) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
		};
	wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);

	queue = wgpuDeviceGetQueue(device);
	return true;
}

void MandelbrotRenderer::Terminate() 
{
	wgpuQueueRelease(queue);
	wgpuSurfaceRelease(surface);
	wgpuDeviceRelease(device);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void MandelbrotRenderer::Tick() {
	glfwPollEvents();


#if defined(WEBGPU_BACKEND_DAWN)
	wgpuDeviceTick(device);
#elif defined(WEBGPU_BACKEND_WGPU)
	wgpuDevicePoll(device, false, nullptr);
#endif
}

bool MandelbrotRenderer::IsRunning() 
{
	return !glfwWindowShouldClose(window);
}
//int main() 
//{
//    auto start = std::chrono::high_resolution_clock::now();
//
//    std::vector<double> buffer(WIDTH * HEIGHT);
//
//
//    std::vector<std::thread> threads; 
//    
//    int c = 0;
//    for (int k = 0; k < THR_COUNT; ++k)
//    {
//        c++;
//        threads.emplace_back(renderRow, k, std::ref(buffer));
//    }
//  
//
//
//
//    std::cout <<"Threads:" << c << std::endl;
//    for (auto& t : threads) t.join();
//
//
//
//    std::ofstream ofs("mandelbrot.ppm");
//    ofs << "P3\n" << WIDTH << " " << HEIGHT << " 255\n";
//    for (double v : buffer)
//    {
//
//
//        double J = v / double(MAX_IT);
//
//        J = pow(J, 0.4);
//
//        float hue = float(fmod(J * 360.0 + 240, 360.0));
//
//        float value = (v < MAX_IT) ? 1.0f : 0.0f;
//
//
//        RGB color = hsvToRgb(hue, 1.0f, value);
//        ofs << color.r << ' ' << color.g << ' ' << color.b << '\n';
//
//    }
//
//    auto end = std::chrono::high_resolution_clock::now();
//
//    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
//
//    std::cout << "Time taken: " << duration << " ms" << std::endl;
//
//
//}

//#else
//
//int main() 
//{
//    auto start = std::chrono::high_resolution_clock::now();
//
//    std::vector<double> buffer(WIDTH * HEIGHT);
//
//    for (int y = 0; y < HEIGHT; ++y) 
//    {
//        for (int x = 0; x < WIDTH; ++x)
//        {
//
//            double cr = (x - WIDTH / 2.0) * 4.0 / WIDTH;
//            double ci = (y - HEIGHT / 2.0) * 4.0 / HEIGHT;
//
//
//            buffer[y * WIDTH + x] = mandelbrot(cr, ci);
//
//        }
//    }
//  
//    std::ofstream ofs("mandelbrot.ppm");
//    ofs << "P3\n" << WIDTH << " " << HEIGHT << " 255\n";
//    for (double v : buffer)
//    {
//
//
//        double J = v / double(MAX_IT);
//    
//        J = pow(J, 0.4);
//
//        float hue = float(fmod(J * 360.0+240, 360.0));
//
//        float value = (v < MAX_IT) ? 1.0f : 0.0f;
//
//
//        RGB color = hsvToRgb(hue, 1.0f, value);
//        ofs << color.r << ' ' << color.g << ' ' << color.b << '\n';
//
//
//    }
//
//    auto end = std::chrono::high_resolution_clock::now();
//
//    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
//
//    std::cout << "Time taken: " << duration << " ms" << std::endl;
//
//
//}
//
//
//
//
//
//
//#endif 