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

struct spotifyData {
    bool success;
    boost::json::value data;
};

boost::json::value errDefault = {
        {"error", "Something Went Wrong With Spotify API"}
};

std::map<std::string, std::string> isValidType(const std::string& url) {
    std::regex urlPattern("https:\\/\\/open\\.spotify\\.com\\/(track|album)\\/([a-zA-Z0-9]+)(\\?.*)?");
    std::smatch match;

    if (std::regex_match(url, match, urlPattern)) {
        return { {"type", match[1]}, {"id", match[2]} };
    }

    return { {"type", "0"}, {"id", "0"} };
}

tokenResult getToken() {
    const std::string clientId = std::getenv("CLIENT_ID");
    const std::string clientSecret = std::getenv("CLIENT_SECRET");

    if (clientId.length() == 0 || clientSecret.length() == 0) {
        std::cout << "Environment Variable Missing" << std::endl;
        exit(1);
    }

    std::string authurl = "https://accounts.spotify.com/api/token";
    cpr::Response r = cpr::Post(cpr::Url{ authurl },
        cpr::Payload{ {"grant_type", "client_credentials"} },
        cpr::Authentication{ clientId, clientSecret, cpr::AuthMode::BASIC });

    if (r.status_code != 200) {
        return { false, "Something Went Wrong while getting access token" };
    }

    boost::json::value json = boost::json::parse(r.text);
    std::string accessToken = boost::json::value_to<std::string>(json.at("access_token"));
    return { true, accessToken };
}

std::map<std::string, std::string, cpr::CaseInsensitiveCompare> getAuthHeader(const std::string& token) {
    return { {"authorization", "Bearer " + token} };
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


void printTrackDetails(boost::json::value trackJson) {
    std::cout << boost::json::value_to<std::string>(trackJson.at("name"));
}


spotifyData getTrackInfo(const std::string& trackId, const std::string& token) {
    std::string endpointURL = "https://api.spotify.com/v1/tracks/" + trackId;
    auto authHeader = getAuthHeader(token);
    cpr::Response res = cpr::Get(cpr::Url{ endpointURL },
        cpr::Header{ authHeader });
    if (res.status_code != 200) {
        return { false , errDefault };
    }
    boost::json::value jsonData = boost::json::parse(res.text);
    return { true, jsonData };

}

void processTracks(const std::string& trackId, const std::string& token) {
    spotifyData sp = getTrackInfo(trackId, token);
    if (sp.success) {
        printTrackDetails(sp.data);
    }

}


int main() {
    dotenv::init(); // Load Environment Variables from .env
    std::map<std::string, std::string> urlType = inputURL();

    tokenResult tokenResult = getToken();

    // Check the result
    if (tokenResult.success) {
        if (urlType["type"] == "track") {
            processTracks(urlType["id"], tokenResult.token);
        }
    }
    else {
        std::cerr << "Error: " << tokenResult.token << std::endl;
    }

    return 0;
}