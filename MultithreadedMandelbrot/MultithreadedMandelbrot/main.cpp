#include <vector>
#include <complex>
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>


#define Multi 1;

constexpr int WIDTH = 1920, HEIGHT = 1920, MAX_IT = 5500;



struct RGB { int r, g, b; };

RGB hsvToRgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;
    float rp, gp, bp;

    if (h < 60) { rp = c; gp = x; bp = 0; }
    else if (h < 120) { rp = x; gp = c; bp = 0; }
    else if (h < 180) { rp = 0; gp = c; bp = x; }
    else if (h < 240) { rp = 0; gp = x; bp = c; }
    else if (h < 300) { rp = x; gp = 0; bp = c; }
    else { rp = c; gp = 0; bp = x; }

    return {
        int((rp + m) * 255),
        int((gp + m) * 255),
        int((bp + m) * 255)
    };
}


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

#if Multi

void renderRow(int y, std::vector<double>& buf)
{
    for (int x = 0; x < WIDTH; ++x)
    {
        double cr = (x - WIDTH / 2.0) * 4.0 / WIDTH;
        double ci = (y - HEIGHT / 2.0) * 4.0 / HEIGHT;
        buf[y * WIDTH + x] = mandelbrot(cr, ci);
    }
}

int main() 
{
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<double> buffer(WIDTH * HEIGHT);


    std::vector<std::thread> threads;
    for (int y = 0; y < HEIGHT; ++y)
        threads.emplace_back(renderRow, y, std::ref(buffer));

    for (auto& t : threads) t.join();



    std::ofstream ofs("mandelbrot.ppm");
    ofs << "P3\n" << WIDTH << " " << HEIGHT << " 255\n";
    for (double v : buffer)
    {


        double J = v / double(MAX_IT);

        J = pow(J, 0.4);

        float hue = float(fmod(J * 360.0 + 240, 360.0));

        float value = (v < MAX_IT) ? 1.0f : 0.0f;


        RGB color = hsvToRgb(hue, 1.0f, value);
        ofs << color.r << ' ' << color.g << ' ' << color.b << '\n';

    }

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Time taken: " << duration << " ms" << std::endl;


}

#else

int main() 
{
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<double> buffer(WIDTH * HEIGHT);

    for (int y = 0; y < HEIGHT; ++y) 
    {
        for (int x = 0; x < WIDTH; ++x)
        {

            double cr = (x - WIDTH / 2.0) * 4.0 / WIDTH;
            double ci = (y - HEIGHT / 2.0) * 4.0 / HEIGHT;


            buffer[y * WIDTH + x] = mandelbrot(cr, ci);

        }
    }
  
    std::ofstream ofs("mandelbrot.ppm");
    ofs << "P3\n" << WIDTH << " " << HEIGHT << " 255\n";
    for (double v : buffer)
    {


        double J = v / double(MAX_IT);
    
        J = pow(J, 0.4);

        float hue = float(fmod(J * 360.0+240, 360.0));

        float value = (v < MAX_IT) ? 1.0f : 0.0f;


        RGB color = hsvToRgb(hue, 1.0f, value);
        ofs << color.r << ' ' << color.g << ' ' << color.b << '\n';


    }

    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Time taken: " << duration << " ms" << std::endl;


}






#endif 