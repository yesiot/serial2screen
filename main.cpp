#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/program_options.hpp>

const uint32_t cDefaultWidth = 96;
const uint32_t cDefaultHeight= 96;
const std::string cDefaultSyncString = "XXSYNCXX";
const std::string cDefaultDev = "/dev/ttyACM0";

namespace po = boost::program_options;

int main(const int argc, const char* argv[])
{
    cv::Mat frame, fgMask, back, dst;

    boost::asio::io_service io;
    boost::system::error_code error;

    po::options_description desc("Options");
    desc.add_options()
            ("help", "produce help message")
            ("dev", po::value<std::string>()->default_value(cDefaultDev), "serial device")
            ("sync-string", po::value<std::string>()->default_value(cDefaultSyncString), "set synchronization string")
            ("width", po::value<uint32_t>()->default_value(cDefaultWidth), "image width")
            ("height", po::value<uint32_t>()->default_value(cDefaultHeight), "image height")

            ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: serial2screen [options]\n";
        std::cout << desc;
        return 0;
    }

    auto syncString = vm["sync-string"].as<std::string>();
    auto serialDevice = vm["dev"].as<std::string>();
    auto imgWidth = vm["width"].as<std::uint32_t>();
    auto imgHeight = vm["height"].as<std::uint32_t>();

    auto port = boost::asio::serial_port(io);
    port.open(serialDevice);
    port.set_option(boost::asio::serial_port_base::baud_rate(115200));

    std::thread t([&io]() {io.run();});

    cv::namedWindow("serial2screen", cv::WINDOW_NORMAL);
    cv::resizeWindow("serial2screen", 240, 240);

    while(true) {

        uint8_t buffer[imgWidth * imgHeight];

        uint8_t syncByte = 0;
        uint8_t currentByte;

        while (true) {

            boost::asio::read(port, boost::asio::buffer(&currentByte, 1));
            if (currentByte == syncString[syncByte]) {
                syncByte++;
            } else {
                std::cerr << (char) currentByte;
                syncByte = 0;
            }
            if (syncByte == syncString.length()) {
                std::cerr << std::endl;
                break;
            }
        }

        boost::asio::read(port, boost::asio::buffer(buffer, imgHeight * imgWidth));

        frame = cv::Mat(imgHeight, imgWidth, CV_8U, buffer);

        if (frame.empty()) {
            std::cerr << "ERROR! blank frame grabbed" << std::endl;
            continue;
        }

        imshow("serial2screen", frame);

        if (cv::waitKey(5) >= 0)
            break;
    }
    return 0;
}