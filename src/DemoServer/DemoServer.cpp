#include <ExgBackE.hpp>
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

    // Initialize the ExchangeBackend
    boost::asio::io_context ioContext;
    ExchangeBackend exchangeBackend(ioContext, address, port, true);

    // Start the ExchangeBackend
    exchangeBackend.start();

    std::cout << "Press Enter to stop the ExchangeBackend..." << std::endl;
    std::cin.get();

    // Stop the ExchangeBackend
    exchangeBackend.stop();

    return 0;
}