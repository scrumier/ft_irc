#include "IRCBot.hpp"

#include <ctime>
#include <iomanip>

void Bot::initialize_trivia() {
    _triviaQuestions["What is the capital of France?"] = "Paris";
    _triviaQuestions["What is the capital of Japan?"] = "Tokyo";
    _triviaQuestions["What is the capital of Italy?"] = "Rome";
    _triviaQuestions["What is the capital of Spain?"] = "Madrid";
    _triviaQuestions["What is the capital of Germany?"] = "Berlin";
    _triviaQuestions["What is the capital of the United States?"] = "Washington";
    _triviaQuestions["What is the capital of Canada?"] = "Ottawa";
    _triviaQuestions["What is the capital of Australia?"] = "Canberra";
    _triviaQuestions["What is the capital of Brazil?"] = "Brasilia";
    _triviaQuestions["What is the capital of India?"] = "New Delhi";
    
    _triviaQuestions["What is the largest planet in our solar system?"] = "Jupiter";
    _triviaQuestions["What is the smallest planet in our solar system?"] = "Mercury";
    _triviaQuestions["What is the largest mammal in the world?"] = "Blue whale";
    _triviaQuestions["What is the largest ocean in the world?"] = "Pacific Ocean";
    _triviaQuestions["What is the largest continent in the world?"] = "Asia";
    _triviaQuestions["What is the largest country in the world?"] = "Russia";
    _triviaQuestions["What is the largest desert in the world?"] = "Antarctica";
    _triviaQuestions["What is the largest mountain in the world?"] = "Mount Everest";
    _triviaQuestions["What is the largest lake in the world?"] = "Caspian Sea";
    _triviaQuestions["What is the largest river in the world?"] = "Nile River";

    _triviaQuestions["What is the largest animal in the world?"] = "Blue whale";
    _triviaQuestions["What is the smallest animal in the world?"] = "Bee hummingbird";
    _triviaQuestions["What is the largest bird in the world?"] = "Ostrich";
    _triviaQuestions["What is the smallest bird in the world?"] = "Bee hummingbird";
    _triviaQuestions["What is the largest fish in the world?"] = "Whale shark";
    _triviaQuestions["What is the smallest fish in the world?"] = "Paedocypris";
    _triviaQuestions["What is the largest reptile in the world?"] = "Saltwater crocodile";
    _triviaQuestions["What is the smallest reptile in the world?"] = "Brookesia micra";
    _triviaQuestions["What is the largest amphibian in the world?"] = "Chinese giant salamander";
    _triviaQuestions["What is the smallest amphibian in the world?"] = "Paedophryne amauensis";
}

void Bot::ask_trivia_question() {
    // Store the keys (questions) in a vector for random selection
    std::vector<std::string> keys;
    for (std::map<std::string, std::string>::iterator it = _triviaQuestions.begin(); it != _triviaQuestions.end(); ++it) {
        keys.push_back(it->first);
    }

    // Select a random question key
    int random_index = std::rand() % keys.size();
    _current_question_index = random_index;
    std::string question = keys[random_index];
    
    // Send the selected question
    std::string response = "PRIVMSG " + _channel + " :" + question;
    send_msg(response);
}

void Bot::check_trivia_answer(const std::string& user_answer) {
    if (_current_question_index == -1) {
        std::string response = "PRIVMSG " + _channel + " :No question has been asked yet!";
        send_msg(response);
        return;
    }
    // Retrieve the question used in `ask_trivia_question`
    std::vector<std::string> keys;
    for (std::map<std::string, std::string>::iterator it = _triviaQuestions.begin(); it != _triviaQuestions.end(); ++it) {
        keys.push_back(it->first);
    }
    std::string current_question = keys[_current_question_index];

    // Check if the answer is correct
    if (user_answer == _triviaQuestions[current_question]) {
        std::string response = "PRIVMSG " + _channel + " :Correct!";
        send_msg(response);
    } else {
        std::string response = "PRIVMSG " + _channel + " :Incorrect. Try again!";
        send_msg(response);
    }
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string parse_weather_response(const std::string& json) {
    std::string description;
    std::string temperature;
    std::string humidity;
    std::string wind_speed;

    size_t desc_pos = json.find("\"description\":\"");
    if (desc_pos != std::string::npos) {
        size_t start = desc_pos + 15; 
        size_t end = json.find("\"", start);
        description = json.substr(start, end - start);
    }

    size_t temp_pos = json.find("\"temp\":");
    if (temp_pos != std::string::npos) {
        size_t start = temp_pos + 7;
        size_t end = json.find(",", start);
        double temp_kelvin = std::atof(json.substr(start, end - start).c_str());
        double temp_celsius = temp_kelvin - 273.15;

        std::ostringstream temp_stream;
        temp_stream << std::fixed << std::setprecision(2) << temp_celsius;
        temperature = temp_stream.str() + "°C";
    }

    size_t hum_pos = json.find("\"humidity\":");
    if (hum_pos != std::string::npos) {
        size_t start = hum_pos + 11;
        size_t end = json.find(",", start);
        humidity = json.substr(start, end - start) + "%";
    }

    size_t wind_pos = json.find("\"speed\":");
    if (wind_pos != std::string::npos) {
        size_t start = wind_pos + 8; // length of "speed": is 8
        size_t end = json.find(",", start);
        wind_speed = json.substr(start, end - start) + " m/s";
    }

    std::string weather_info = "\nWeather: " + description +
                               "\nTemp: " + temperature +
                               "\nHumidity: " + humidity +
                               "\nWind: " + wind_speed;
    return weather_info;
}

void Bot::fetch_weather(const std::string& location) {
    std::string api_key = "72bb9dab46b9ec3d65f423c63f27a9b8";
    std::string host = "146.185.152.20";
    std::string request_url = "/data/2.5/weather?q=" + location + "&appid=" + api_key;

    std::string msg = "PRIVMSG " + _channel + " :Fetching weather data for " + location + "...";
    send_msg(msg);

    struct addrinfo hints;
    struct addrinfo* res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), "80", &hints, &res) != 0) {
        std::cerr << "Failed to resolve host" << std::endl;
        return;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        freeaddrinfo(res);
        return;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        std::cerr << "Failed to connect to server" << std::endl;
        close(sockfd);
        freeaddrinfo(res);
        return;
    }
    freeaddrinfo(res);

    std::string request = "GET " + request_url + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n\r\n";

    send(sockfd, request.c_str(), request.size(), 0);

    char buffer[4096];
    std::string response;
    ssize_t bytes_received;
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        response += buffer;
    }

    close(sockfd);

    size_t json_pos = response.find("\r\n\r\n");
    if (json_pos != std::string::npos) {
        std::string json_response = response.substr(json_pos + 4); // Skip the header section

        std::string clean_response = parse_weather_response(json_response);

        std::string reply = "PRIVMSG " + _channel + " :Weather data: " + clean_response;
        send_msg(reply);
    } else {
        std::cerr << "Failed to parse weather response" << std::endl;
    }
}

std::string chatToString(const char* chat) {
    std::string str(chat);
    return str;
}

void Bot::handle_server_message(const std::string& message) {
    if (message.find("001") != std::string::npos) {
        send_msg("JOIN " + _channel);
    }

    if (message.find("PRIVMSG") != std::string::npos) {
        if (message.find("!hello") != std::string::npos) {
            std::cout << "Handling hello message" << std::endl;
            send_msg("PRIVMSG " + _channel + " :world!");
        }
        else if (message.find("!trivia") != std::string::npos) {
            std::cout << "Handling trivia message" << std::endl;
            ask_trivia_question();
        }
        else if (message.find("!answer ") != std::string::npos) {
            std::cout << "Handling answer message" << std::endl;
            std::string user_answer = message.substr(message.find("!answer ") + 8);
            check_trivia_answer(user_answer);
        }
        else if (message.find("!weather ") != std::string::npos) {
            std::cout << "Handling weather message" << std::endl;
            std::string location = message.substr(message.find("!weather ") + 9);
            fetch_weather(location);
        }
    }
}
