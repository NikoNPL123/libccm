#include <iostream>
#include <libzippp.h>
#include <vector>
#include <map>
#include <fstream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

class CustomCampaignManager {
    private:
        std::string sc2Root; //root directory of Starcraft 2
        std::string customCampaignsFolder; //folder, where custom campaigns are stored (relative to sc2Root)
        std::string activeCampaignFolder; //folder where active campaign is (relative to sc2Root)
        fs::path appPath; //root path of this app
        std::string lastInstalledCampaignPath; //path of latest installed campaign

        std::map<std::string, std::string> metadata;

        //we want this class to avoid typos in expansions
        class Expansion {
            public: 
                std::string expansion; //"WoL" for Wings of Liberty, "HotS" for heart of the swarm, "LotV" for Legacy of the Void, "NCO" for Nova Covert Ops
                std::string path; //folder where active campaign for this expansion is (relative to activeCampaignFolder)
        };

        bool prepareSelectedEntryFolder(std::string); //creates directory for archive entry

    public: 
        Expansion WoL;
        Expansion HotS;
        Expansion LotV;
        Expansion NCO;

        CustomCampaignManager(std::string = "Maps/CustomCampaigns/", std::string = "Maps/Campaign/"); //You shouldn't change default values here. It will work with different value for custom campaigns, but it won't be compatibile with original Custom Campaign Manager, and won't work for other active campaign dir

        bool unpackCampaign(std::string); //unpacks campaign to ./tmp folder
        bool installUnpackedCampaign(); //installs unpacked campaign to customCampaignsFolder
        bool findDefaultRootDir(); //tries to find Stracraft II in it's default directory. Returns true if it finds it, or false when it doesn't
        bool prepareCustomCampaignsFolder(); //checks if custom campaigns folder exist. If no, it creates one
        bool prepareActiveCampaignsFolders(); //checks if active campaign folder exist. IF no, it creates one
        std::map<std::string, fs::path> getCampaignsList(); //gets custom campaigns list from customCampaignsFolder. <key> is name + version, <value> is absolute path to campaign root folder
        std::map<std::string, std::map<std::string, std::string>> getFullCampaignsList(); //gets custom campaigns list from customCampaignsFolder. <key> is path, <value> is metadata map
        std::map<std::string, std::string> getActiveCampaignData(Expansion&); //gets active campaign data. <key> is key from metadata.txt, <value> is value from metadata.txt.
        
        bool setActiveCampaign(fs::path); //set current active campaign
        bool restartCampaigns(); //deletes all active campaigns folders and creates empty ones
        bool restartExpansion(Expansion&); //deletes <Expansion> active campaign

        std::string getMetadataKey(std::string); //returns value of key from map metadata. If key doesn't exist, it will return empty string
        bool loadMetadataFile(std::string); //loads metadata from file into metadata map

        bool setSc2Root(std::string); //sets sc2Root variable. You should pass absolute path to it
        void setCustomCampaignsFolder(std::string); //sets custom campaigns folder. You should pass relative path to it
        void setActiveCampaignFolder(std::string); //sets active campaign folder. You should pass relative path to it

        std::string getSc2Root();
        std::string getCustomCampaignsFolder();
        std::string getActiveCampaignFolder();
        std::string getLastInstalledCampaignPath();
};