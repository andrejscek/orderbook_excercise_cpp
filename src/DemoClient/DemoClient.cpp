#include <InpStrm.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    std::string address = "127.0.0.1";
    int port = 1234;

    // Define and parse command-line options
    po::options_description desc("Allowed options");
    desc.add_options()("help", "Produce help message")("address,a", po::value<std::string>(&address)->default_value(address), "ExchangeBackend address")("port,p", po::value<int>(&port)->default_value(port), "ExchangeBackend port");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }

    if (vm.count("address"))
    {
        address = vm["address"].as<std::string>();
    }

    if (vm.count("port"))
    {
        port = vm["port"].as<int>();
    }

    boost::asio::io_service io_service;
    udp::resolver resolver(io_service);
    udp::resolver::query query(udp::v4(), address, std::to_string(port));
    udp::endpoint receiver_endpoint = *resolver.resolve(query);

    // udp::socket socket(io_service);
    udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));

    // std::string input = "N, 1, IBM, 10, 100, B, 1";
    std::string input;

    std::cout << "Enter a message to send (or 'quit' to exit):" << std::endl;
    while (std::getline(std::cin, input) && input != "quit")
    {
        if (parseAndSend(input, socket, receiver_endpoint))
        {
            std::cout << "Sent message: " << input << std::endl;
        }
        else
        {
            std::cerr << "Failed to send message: " << input << std::endl;
        }
    }
    socket.close();
    return 0;
}