#include <vector>
#include <complex>
#include <fstream>
#include <thread>
constexpr int WIDTH = 600, HEIGHT = 600, MAX_IT = 500;



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

int main() 
{
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




      /*  float J = (float)v / (float)MAX_IT;

      
        int r = J * 255;
        int g = J * 255;
        int b = J * 255;

        ofs << r  << ' ' << g << ' ' << b << '\n';*/


        double J = v / double(MAX_IT);

        float hue =  J*360;
        RGB Color;
        Color = hsvToRgb(240+hue, 1.0f, J != 1.0f ? 1.0f : 0.0f);
     //   Color = hsvToRgb(hue, 1.0f, 0.2f);

        ofs << Color.r << ' ' << Color.g << ' ' << Color.b << '\n';


    }
}


//void renderRow(int y, std::vector<int>& buf) {
//    for (int x = 0; x < WIDTH; ++x) {
//        double cr = (x - WIDTH / 2.0) * 4.0 / WIDTH;
//        double ci = (y - HEIGHT / 2.0) * 4.0 / HEIGHT;
//        buf[y * WIDTH + x] = mandelbrot(cr, ci);
//    }
//}
//
//int main() {
//    std::vector<int> buffer(WIDTH * HEIGHT);
//    std::vector<std::thread> threads;
//    for (int y = 0; y < HEIGHT; ++y)
//        threads.emplace_back(renderRow, y, std::ref(buffer));
//    for (auto& t : threads) t.join();
//
//    std::ofstream ofs("mandelbrot.ppm");
//    ofs << "P3\n" << WIDTH << " " << HEIGHT << " 255\n";
//    for (int v : buffer) ofs << (v % 256) << ' ' << (v % 256) << ' ' << (v % 256) << '\n';
//}