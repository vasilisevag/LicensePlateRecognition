#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <cmath>
#include <unordered_map>
#include <cstdio>
using namespace std;

const unsigned int H = 3120;
const unsigned int W = 4160;
const unsigned int HOR_WINDOW_SIZE = 5;
const unsigned int VER_WINDOW_SIZE = 1;
const unsigned int LICENSE_PLATE_VERTICAL_LOWER_LIMIT = 40;
const double LICENSE_PLATE_ASPECT_RATIO_LOWER_LIMIT = 3.5;
const double LICENSE_PLATE_ASPECT_RATIO_UPPER_LIMIT = 5;
const double LICENSE_PLATE_ASPECT_RATIO = 4.5;
bool image[H][W];

void LoadImageFromFile(const string &filename)
{
    ifstream in("/home/vasilis/Programming/C++ Projects/LPR/" + filename);
    cout << in.bad();
    for (int h = 0; h < H; h++)
        for (int w = 0; w < W; w++){
            in >> image[h][w];
        }
    in.close();
}

vector<vector<pair<int, int>>> GetVerticalLines()
{
    vector<vector<pair<int, int>>> verticalLines(W);
    unsigned int startingPoint, endPoint;
    unsigned int h;
    for (int w = 30; w < W - 30; w++)
    {
        h = 0;
        while (h < H)
        {
            while (h < H && image[h][w] == 0)
                h++;
            startingPoint = h;
            while (h < H && image[h][w] == 1)
                h++;
            endPoint = h;
            if (endPoint - startingPoint > LICENSE_PLATE_VERTICAL_LOWER_LIMIT)
                verticalLines[w].push_back({startingPoint, endPoint});
        }
    }
    return verticalLines;
}

vector<vector<pair<int, int>>> GetLicensePlatesCandidateLocations(const vector<vector<pair<int, int>>> &verticalLines)
{
    vector<vector<pair<int, int>>> licensePlatesCandidates;
    for (int w = 5; w < W; w++) {
        for (const auto &verticalLine : verticalLines[w]) {
            int verticalLineWidth = verticalLine.second - verticalLine.first;
            int horizontalLineWidthLower = w + verticalLineWidth * LICENSE_PLATE_ASPECT_RATIO_LOWER_LIMIT;
            int horizontalLineWidthUpper = w + verticalLineWidth * LICENSE_PLATE_ASPECT_RATIO_UPPER_LIMIT;

            if(horizontalLineWidthLower >= W) continue;
            if(horizontalLineWidthUpper >= W) horizontalLineWidthUpper = W - 1;

            double bestSimilarityMeasurementSoFar = 0;
            bool firstTime = true;
            for (int wr = horizontalLineWidthLower; wr <= horizontalLineWidthUpper; wr++) {
                for (const auto &verticalLineMatch : verticalLines[wr]) {
                
                    if (verticalLineMatch.first - verticalLine.first > verticalLineWidth) break; // you can add a constant there
                    if (verticalLine.first - verticalLineMatch.first > verticalLineWidth) continue; // and there!
                    
                    int matcherVerticalLineWidth = verticalLineMatch.second - verticalLineMatch.first;
                    if(verticalLineWidth / float(matcherVerticalLineWidth) <= 0.8 || 1.25 <= verticalLineWidth / float(matcherVerticalLineWidth))
                        continue; // we can change the confidence level if we like!
                    
                    double similarityMeasurement = 100 // - 0.1*abs(verticalLine.first - verticalLineMatch.first)
                                                 - 0.5*abs(verticalLineWidth - matcherVerticalLineWidth)
                                                 - 0.3*abs(LICENSE_PLATE_ASPECT_RATIO - (double)(wr-w)/((verticalLineWidth + matcherVerticalLineWidth)/2.0));

                    if(similarityMeasurement > bestSimilarityMeasurementSoFar){
                        bestSimilarityMeasurementSoFar = similarityMeasurement;
                        if(firstTime){
                            licensePlatesCandidates.push_back({{verticalLine.first, w}, {verticalLineMatch.first, wr}, {verticalLine.second, w}, {verticalLineMatch.second, wr}});
                            firstTime = false;
                        }
                        else{
                            licensePlatesCandidates[licensePlatesCandidates.size()-1] = {{verticalLine.first, w}, {verticalLineMatch.first, wr}, {verticalLine.second, w}, {verticalLineMatch.second, wr}};
                        }
                    }
                }
            }
        }
    }
    return licensePlatesCandidates;
}

const int CONF_LEVEL = 10;

vector<vector<pair<int, int>>> FilterLicensePlatesCandidates(const vector<vector<pair<int, int>>>& lpc){
    vector<float> metric(lpc.size());
    vector<vector<pair<int, int>>> filteredLicensePlatesCandidates;
    unordered_map<int, int> rejectedLicensePlates;

    for(int i = 0; i < lpc.size(); i++){
        if(rejectedLicensePlates.find(i) != rejectedLicensePlates.end()) continue;
        for(int j = 0; j < lpc.size(); j++){
            if(i == j) continue;
            if(rejectedLicensePlates.find(j) != rejectedLicensePlates.end()) continue;
            bool isInside = true;
            for(int k = 0; k < 4; k++){
                if(!(lpc[i][0].second - CONF_LEVEL <= lpc[j][k].second && lpc[j][k].second <= lpc[i][1].second + CONF_LEVEL)){ // this 20 constant is sus :)
                    isInside = false;
                    break;
                }
                int up = lpc[i][0].first + (lpc[i][1].first - lpc[i][0].first)*(lpc[j][k].second - (float)lpc[i][0].second)/(lpc[i][1].second - lpc[i][0].second);
                int down = lpc[i][2].first + (lpc[i][3].first - lpc[i][2].first)*(lpc[j][k].second - (float)lpc[i][2].second)/(lpc[i][3].second - lpc[i][2].second);
                if(!(up - CONF_LEVEL <= lpc[j][k].first && lpc[j][k].first <= down + CONF_LEVEL)){
                    isInside = false;
                    break;
                }
            }
            if(isInside)
                rejectedLicensePlates[j] = j;   
        }
    }

    for(int i = 0; i < lpc.size(); i++)
        if(rejectedLicensePlates.find(i) == rejectedLicensePlates.end())
            filteredLicensePlatesCandidates.push_back(lpc[i]);
    
    return filteredLicensePlatesCandidates;
}

/************************************************************************************************************************************************/

auto GetLicensePlatesCandidates(){
    
}

int main() {
    LoadImageFromFile("boundaries2");
    auto lisencePlatesCandidates = GetLicensePlatesCandidates();
}