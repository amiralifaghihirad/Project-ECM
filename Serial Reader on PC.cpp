#include <libserialport.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <ctime>
#include <csignal>
#include <iomanip>
#include <stdexcept>

/**
 * RAII wrapper for sp_port to ensure proper cleanup.
 */
struct PortDeleter {
    void operator()(sp_port* port) const {
        if (port) {
            sp_close(port);
            sp_free_port(port);
        }
    }
};
using UniquePort = std::unique_ptr<sp_port, PortDeleter>;

/**
 * RAII wrapper for sp_port_config to ensure proper cleanup.
 */
struct ConfigDeleter {
    void operator()(sp_port_config* config) const {
        if (config) {
            sp_free_config(config);
        }
    }
};
using UniqueConfig = std::unique_ptr<sp_port_config, ConfigDeleter>;

/**
 * Checks for libserialport errors and throws exception with message.
 * @param result The return code from a libserialport function.
 * @param context Context message for error reporting.
 * @throws std::runtime_error if result indicates an error.
 */
void check_error(enum sp_return result, const std::string& context) {
    if (result < 0) {
        char* error_message = nullptr;
        sp_get_error_message(&error_message);
        std::string message = error_message ? error_message : "Unknown error";
        if (error_message) {
            sp_free_error_message(error_message);
        }
        throw std::runtime_error(context + (context.empty() ? "" : ": ") + message);
    }
}

/**
 * Gets current timestamp as string for logging.
 * @return Formatted timestamp (e.g., "2025-06-04 13:00:00") or "Invalid time" on failure.
 */
std::string getTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[20] = "Invalid time";
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now)) == 0) {
        return "Invalid time";
    }
    return buf;
}

/**
 * Checks if a port exists in the system.
 * @param port_name Name of the port to check (e.g., "COM3").
 * @return true if port exists, false otherwise.
 */
bool port_exists(const std::string& port_name) {
    sp_port** ports = nullptr;
    enum sp_return result = sp_list_ports(&ports);
    if (result != SP_OK || !ports) {
        return false;
    }
    bool found = false;
    for (int i = 0; ports[i] != nullptr; ++i) {
        if (port_name == sp_get_port_name(ports[i])) {
            found = true;
            break;
        }
    }
    sp_free_port_list(ports);
    return found;
}

/**
 * Lists available serial ports.
 * @return String containing available port names.
 */
std::string list_ports() {
    sp_port** ports = nullptr;
    enum sp_return result = sp_list_ports(&ports);
    if (result != SP_OK || !ports) {
        return "No ports available";
    }
    std::string port_list = "Available ports: ";
    for (int i = 0; ports[i] != nullptr; ++i) {
        port_list += sp_get_port_name(ports[i]);
        if (ports[i + 1] != nullptr) {
            port_list += ", ";
        }
    }
    sp_free_port_list(ports);
    return port_list;
}

// Signal handler for graceful exit
volatile sig_atomic_t running = 1;
void signal_handler(int sig) {
    running = 0;
}

/**
 * Main function to read serial data from Arduino, parse sensor values, and log to CSV.
 * Expects data format: "Sensor values: gas=%d, temp=%d, s3=%d" or AT/HTTP responses.
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments: [1] port, [2] CSV file, [3] baud rate.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
    // Register signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    try {
        // Parse command-line arguments
        const std::string port_name = (argc > 1) ? argv[1] : "COM3";
        const std::string csv_filename = (argc > 2) ? argv[2] : "sensor_data.csv";
        int baud_rate = (argc > 3) ? std::atoi(argv[3]) : 9600;

        // Validate arguments
        if (argc > 4 || (argc > 3 && baud_rate <= 0)) {
            std::cerr << "Usage: " << argv[0] << " [port] [csv_file] [baud_rate]\n"
                      << "Example: " << argv[0] << " COM3 sensor_data.csv 9600\n"
                      << "Defaults: port=COM3, csv_file=sensor_data.csv, baud_rate=9600\n";
            return 1;
        }

        // Check if port exists
        if (!port_exists(port_name)) {
            std::cerr << "Port " << port_name << " not found. " << list_ports() << std::endl;
            return 1;
        }

        // Open CSV file
        std::ofstream csv_file(csv_filename, std::ios::app);
        if (!csv_file.is_open()) {
            std::cerr << "Failed to open CSV file: " << csv_filename << std::endl;
            return 1;
        }
        csv_file.seekp(0, std::ios::end);
        if (csv_file.tellp() == 0) {
            csv_file << "Timestamp,Gas,Temp,S3\n";
        }

        // Initialize serial port
        UniquePort port(nullptr);
        {
            sp_port* raw_port = nullptr;
            check_error(sp_get_port_by_name(port_name.c_str(), &raw_port), "Finding port");
            port.reset(raw_port);
        }
        check_error(sp_open(port.get(), SP_MODE_READ), "Opening port");

        UniqueConfig config(nullptr);
        {
            sp_port_config* raw_config = nullptr;
            check_error(sp_new_config(&raw_config), "Creating config");
            config.reset(raw_config);
        }
        check_error(sp_set_config_baudrate(config.get(), baud_rate), "Setting baud rate");
        check_error(sp_set_config_bits(config.get(), 8), "Setting data bits");
        check_error(sp_set_config_parity(config.get(), SP_PARITY_NONE), "Setting parity");
        check_error(sp_set_config_stopbits(config.get(), 1), "Setting stop bits");
        check_error(sp_set_config(port.get(), config.get()), "Applying config");

        // Print startup message
        std::cout << "----------------------------------------\n"
                  << "Serial Reader started\n"
                  << "Port: " << port_name << "\n"
                  << "Baud rate: " << baud_rate << "\n"
                  << "Logging to: " << csv_filename << "\n"
                  << "Press Ctrl+C to exit\n"
                  << "----------------------------------------\n";

        // Serial reading variables
        char buffer[256];
        std::string accumulated_data;
        const unsigned long RECONNECT_INTERVAL = 5000; // Retry every 5s
        unsigned long lastReconnectAttempt = 0;
        const size_t MAX_ACCUMULATED_SIZE = 4096; // Max accumulated data size
        bool port_open = true;
        auto last_data_time = std::chrono::steady_clock::now();
        const auto INCOMPLETE_LINE_TIMEOUT = std::chrono::seconds(10);

        while (running) {
            // Check CSV file status
            if (!csv_file.good()) {
                std::cerr << getTimestamp() << " CSV file error, attempting to reopen...\n";
                csv_file.close();
                csv_file.open(csv_filename, std::ios::app);
                if (!csv_file.is_open()) {
                    std::cerr << getTimestamp() << " Failed to reopen CSV file!\n";
                } else if (csv_file.tellp() == 0) {
                    csv_file << "Timestamp,Gas,Temp,S3\n";
                }
            }

            // Read data from serial port if open
            int bytes_read = 0;
            if (port_open) {
                try {
                    bytes_read = sp_blocking_read(port.get(), buffer, sizeof(buffer) - 1, 1000);
                } catch (const std::exception& e) {
                    std::cerr << getTimestamp() << " Serial read exception: " << e.what() << "\n";
                    bytes_read = -1; // Trigger reconnect
                }
            }

            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';

                // Check if adding data exceeds max size
                if (accumulated_data.size() + bytes_read > MAX_ACCUMULATED_SIZE) {
                    std::cerr << getTimestamp() << " Warning: Accumulated data too large, clearing!\n";
                    accumulated_data.clear();
                }
                accumulated_data.append(buffer, bytes_read);
                last_data_time = std::chrono::steady_clock::now();

                // Process complete lines
                size_t pos;
                while ((pos = accumulated_data.find('\n')) != std::string::npos) {
                    std::string line = accumulated_data.substr(0, pos);
                    accumulated_data.erase(0, pos + 1);

                    // Remove trailing \r or \n
                    while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
                        line.pop_back();
                    }

                    // Process non-empty lines
                    if (!line.empty()) {
                        std::string timestamp = getTimestamp();
                        if (line.find("Sensor values") != std::string::npos) {
                            int gas, temp, s3;
                            if (sscanf(line.c_str(), "Sensor values: gas=%d, temp=%d, s3=%d", &gas, &temp, &s3) == 3) {
                                // Validate sensor values
                                bool valid = true;
                                if (gas < 0 || gas > 1023 || temp < 0 || temp > 1023 || s3 < 0 || s3 > 1023) {
                                    std::cerr << timestamp << " Warning: Invalid sensor values - Gas: " << gas
                                              << ", Temp: " << temp << ", S3: " << s3 << "\n";
                                    valid = false;
                                }
                                if (valid) {
                                    std::cout << timestamp << " Sensor | Gas: " << std::setw(4) << gas
                                              << ", Temp: " << std::setw(4) << temp
                                              << ", S3: " << std::setw(4) << s3 << "\n";
                                    if (csv_file.good()) {
                                        csv_file << timestamp << "," << gas << "," << temp << "," << s3 << "\n";
                                        csv_file.flush();
                                    }
                                }
                            } else {
                                std::cout << timestamp << " Invalid Sensor Data: " << line << "\n";
                            }
                        } else if (line.find("OK") != std::string::npos ||
                                   line.find("ERROR") != std::string::npos ||
                                   line.find("HTTP") != std::string::npos) {
                            std::cout << timestamp << " Response | " << line << "\n";
                        } else {
                            std::cout << timestamp << " Data     | " << line << "\n";
                        }
                    }
                }
            } else if (bytes_read < 0 && port_open) {
                unsigned long now = std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch()).count();
                if (now - lastReconnectAttempt >= RECONNECT_INTERVAL) {
                    std::cerr << getTimestamp() << " Error reading from port, reconnecting...\n";
                    sp_close(port.get());
                    port_open = false;
                    try {
                        check_error(sp_open(port.get(), SP_MODE_READ), "Reopening port");
                        check_error(sp_set_config(port.get(), config.get()), "Reapplying config");
                        std::cout << getTimestamp() << " Reconnected to " << port_name << "\n";
                        port_open = true;
                    } catch (const std::exception& e) {
                        std::cerr << getTimestamp() << " Reconnect failed: " << e.what() << ", retrying in "
                                  << RECONNECT_INTERVAL / 1000 << "s\n";
                    }
                    lastReconnectAttempt = now;
                }
            }

            // Check for incomplete lines timeout
            if (!accumulated_data.empty()) {
                auto now = std::chrono::steady_clock::now();
                if (now - last_data_time > INCOMPLETE_LINE_TIMEOUT) {
                    std::cerr << getTimestamp() << " Warning: Incomplete line timed out, clearing buffer\n";
                    accumulated_data.clear();
                    last_data_time = now;
                }
            }

            // Sleep to reduce CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Cleanup
        std::cout << getTimestamp() << " Exiting gracefully...\n";
        if (csv_file.is_open()) {
            csv_file.close();
        }
    } catch (const std::exception& e) {
        std::cerr << getTimestamp() << " Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << getTimestamp() << " Unknown fatal error occurred\n";
        return 1;
    }

    return 0;
}