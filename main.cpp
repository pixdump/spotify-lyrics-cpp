#include <cpr/cpr.h>
#include <iostream>
#include <boost/json/src.hpp>
#include <dotenv.h>
#include <string>
#include <regex>


struct tokenResult {
    bool success;
    std::string token;
};

struct authHeader {
    std::string auth;
    std::string value;
};

std::map<std::string, std::string> isValidType(const std::string& url) {
    std::regex urlPattern("https://open\\.spotify\\.com/(track|album)/([a-zA-Z0-9]+)");
    std::smatch match;

    if (std::regex_match(url, match, urlPattern)) {
        return { {"type", match[1]}, {"id", match[2]} };
    }

    return { {"type", "0"}, {"id", "0"} };
}

tokenResult getToken() {
    const std::string clientId = std::getenv("CLIENT_ID");
    const std::string clientSecret = std::getenv("CLIENT_SECRET");
    std::string authurl = "https://accounts.spotify.com/api/token";
    cpr::Response r = cpr::Post(cpr::Url{ authurl },
        cpr::Payload{ {"grant_type", "client_credentials"} },
        cpr::Authentication{ clientId, clientSecret, cpr::AuthMode::BASIC });

    if (r.status_code != 200) {
        return { false, "Something Went Wrong while getting access token" };
    }

    boost::json::value json = boost::json::parse(r.text);

    std::string accessToken = json.at("access_token").as_string().c_str();
    return { true, accessToken };
}

authHeader getAuthHeader(const std::string& token) {
    return { "authorization" , "Bearer " + token };
}

std::map<std::string, std::string> inputURL() {
    std::string url;
    std::cout << "Enter Spotify URL: ";
    std::cin >> url;
    std::map<std::string, std::string> result = isValidType(url);
    if (result["type"] != "0") {
        std::cout << "URL is valid." << std::endl;
        return { {"type",result["type"]}, {"id", result["id"]} };
    }
    return inputURL();
}


int main() {
    dotenv::init(); // Load Environment Variables from .env
    inputURL();
    tokenResult tokenResult = getToken();

    // Check the result
    if (tokenResult.success) {
        // std::cout << "Token: " << tokenResult.token << std::endl;
        authHeader res = getAuthHeader(tokenResult.token);
        std::cout << "{" << res.auth << ":" << res.value << std::endl;
    }
    else {
        std::cerr << "Error: " << tokenResult.token << std::endl;
    }

    return 0;
}