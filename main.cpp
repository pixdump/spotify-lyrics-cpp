#include <cpr/cpr.h>
#include <iostream>
#include<boost/json/src.hpp>
#include<dotenv.h>
#include<string>

struct tokenResult {
    bool success;
    std::string token;
};

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

int main() {
    // Call the function
    dotenv::init();
    tokenResult tokenResult = getToken();

    // Check the result
    if (tokenResult.success) {
        std::cout << "Token: " << tokenResult.token << std::endl;
    }
    else {
        std::cerr << "Error: " << tokenResult.token << std::endl;
    }

    return 0;
}