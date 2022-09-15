#include "CustomCampaignManager.h"

namespace fs = boost::filesystem;

CustomCampaignManager::CustomCampaignManager(std::string ccFolder, std::string acFolder) {
    appPath = fs::current_path();
    setCustomCampaignsFolder(ccFolder);
    setActiveCampaignFolder(acFolder);

    WoL.expansion = "WoL";
    WoL.path = "";
    HotS.expansion = "HotS";
    HotS.path = "swarm/";
    LotV.expansion = "Lotv";
    LotV.path = "void/";
    NCO.expansion = "NCO";
    NCO.path = "nova/";
}

bool CustomCampaignManager::unpackCampaign(std::string filedir) {
    fs::current_path(appPath);

    if(fs::is_directory((fs::path)"./tmp")) {
        fs::remove_all((fs::path)"./tmp");
        fs::create_directories((fs::path)"./tmp");
    }
    else {
        fs::create_directories((fs::path)"./tmp");
    }

    libzippp::ZipArchive archive(filedir);
    if(!archive.open(libzippp::ZipArchive::ReadOnly)) {
        std::cout<<std::endl<<"    Archive: "<<filedir<<std::endl;
        return false;
    }

    std::vector<libzippp::ZipEntry> entries = archive.getEntries();
    std::vector<libzippp::ZipEntry>::iterator it;

    fs::current_path((fs::path)("./tmp"));

    for(it=entries.begin() ; it!=entries.end(); ++it) {
        libzippp::ZipEntry entry = *it;

        fs::path a(entry.getName());

        prepareSelectedEntryFolder("/" + a.parent_path().string());

        std::string name = entry.getName();
        int size = entry.getSize();


        void* binaryData = entry.readAsBinary();

        std::fstream file;
        if(name[name.length()-1] != '/' && name[name.length()-1] != '\\') {
            file.open(name, std::ios::out | std::ios::binary);
            file.write((char *) binaryData, size);
            file.close();
        }
        
    }

    archive.close();
    return true;
}

bool CustomCampaignManager::installUnpackedCampaign() {
    const auto copyOptions = fs::copy_options::overwrite_existing
        | fs::copy_options::recursive;
    
    fs::current_path(appPath);
    fs::current_path((fs::path)("./tmp"));

    fs::path modRoot;
    std::string title;

    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(fs::current_path())) {
        if(dir_entry.path().filename().string() == "metadata.txt") {
            loadMetadataFile(fs::absolute(dir_entry.path()).string());
            modRoot = fs::absolute(dir_entry.path().parent_path());
            break;
        }
    }

    title = getMetadataKey("title");

    if(title.empty()) {
        return false;
    }


    fs::path targetPath(sc2Root + customCampaignsFolder + title);

    fs::remove_all(targetPath);
    fs::create_directories(targetPath);

    fs::current_path(modRoot);

    for (const fs::directory_entry& dir_entry : fs::directory_iterator(fs::current_path())) {
        fs::copy(fs::absolute(dir_entry.path()), (fs::path)(targetPath.string() + "/" + dir_entry.path().filename().string()), copyOptions);
        if(dir_entry.path().filename().string() == "metadata.txt") {
            lastInstalledCampaignPath = targetPath.string();
        }
    }

    return true;
}

bool CustomCampaignManager::findDefaultRootDir() {
    std::string dir = "";
    std::string tmpDir = "";

    #ifdef _WIN32
    tmpDir = "C:\\Program Files (x86)\\StarCraft II\\";
    fs::path scPath(tmpDir);
    if(fs::is_directory(scPath)) {
        sc2Root = tmpDir;
        return true;
    }
    tmpDir = "C:\\Program Files\\StarCraft II\\";
    fs::path sc2Path(tmpDir);
    if(fs::is_directory(sc2Path)) {
        sc2Root = tmpDir;
        return true;
    }
    else {
        return false;
    }
    #elif __linux__
    dir = std::getenv("HOME");
    //for debugging purposes
    //dir += "/.packages/.opt/prefixes/starcraft2/drive_c/Program Files (x86)/StarCraft II/";
    dir += "/Games/starcraft-ii/drive_c/Program Files (x86)/StarCraft II/";
    fs::path scPath(dir);
    if(!fs::is_directory(scPath)) {
        return false;
    }
    #elif __APPLE__
    dir = "/Applications/StarCraft II/";
    fs::path scPath(dir);
    if(!fs::is_directory(scPath)) {
        return false;
    }
    #endif

    sc2Root = dir;
    return true;
}

bool CustomCampaignManager::prepareCustomCampaignsFolder() {
    fs::create_directories((fs::path)(sc2Root + customCampaignsFolder));
    return true;
}

bool CustomCampaignManager::prepareActiveCampaignsFolders() {
    fs::create_directories((fs::path)(sc2Root + activeCampaignFolder));
    fs::current_path((fs::path)(sc2Root + activeCampaignFolder));
    fs::create_directories("./swarm");
    fs::create_directories("./void");
    fs::create_directories("./voidprologue");
    fs::create_directories("./nova");
    return true;
}

bool CustomCampaignManager::prepareSelectedEntryFolder(std::string entryFolder) {
    if(entryFolder.empty()) {
        return false;
    }
    fs::path scPath(fs::current_path().string() + entryFolder);
    fs::create_directories(scPath);
    return true;
}

std::map<std::string, fs::path> CustomCampaignManager::getCampaignsList() {
    std::map<std::string, fs::path> map;

    fs::current_path((fs::path)(sc2Root + customCampaignsFolder));
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(fs::current_path())) {
        if(dir_entry.path().filename().string() == "metadata.txt") {
            loadMetadataFile(fs::absolute(dir_entry.path()).string());
            std::string c = getMetadataKey("title") + "[v. " + getMetadataKey("version") + "]";
            map[c] = fs::absolute(dir_entry.path().parent_path());
        }
    }
    return map;
}

std::map<std::string, std::map<std::string, std::string>> CustomCampaignManager::getFullCampaignsList() {
    std::map<std::string, std::map<std::string, std::string>> map;

    fs::current_path((fs::path)(sc2Root + customCampaignsFolder));
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(fs::current_path())) {
        if(dir_entry.path().filename().string() == "metadata.txt") {
            loadMetadataFile(fs::absolute(dir_entry.path()).string());
            std::map<std::string, std::string> tmpMap;
            for(const auto& [key, value] : metadata) {
                tmpMap[key] = value;
            }
            map[fs::absolute(dir_entry.path().parent_path()).string()] = tmpMap;
        }
    }
    return map;
}

std::map<std::string, std::string> CustomCampaignManager::getActiveCampaignData(Expansion& expansion) {
    metadata.clear();
    fs::current_path((fs::path)(sc2Root + activeCampaignFolder + expansion.path));
    for (const fs::directory_entry& dir_entry : fs::directory_iterator(fs::current_path())) {
        if(dir_entry.path().filename().string() == "metadata.txt") {
            loadMetadataFile(fs::absolute(dir_entry.path()).string());
        }
    }
    return metadata;
}

bool CustomCampaignManager::setActiveCampaign(fs::path dir) {
    const auto copyOptions = fs::copy_options::overwrite_existing
        | fs::copy_options::recursive;

    fs::current_path(dir);
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(fs::current_path())) {
        if(dir_entry.path().filename().string() == "metadata.txt") {
            loadMetadataFile(fs::absolute(dir_entry.path()).string());
            break;
        }
    }

    std::string expansion = getMetadataKey("campaign");

    if(expansion == "WoL") {
        restartExpansion(WoL);
        fs::current_path(dir);
        for (const fs::directory_entry& dir_entry : fs::directory_iterator(fs::current_path())) {
            if(dir_entry.path().extension().string() == ".SC2Mod" || dir_entry.path().extension().string() == ".sc2mod") {
                fs::copy(dir_entry.path(), (fs::path)(sc2Root + "Mods/" + dir_entry.path().filename().string()), copyOptions);
            }
            else {
                fs::copy(dir_entry.path(), (fs::path)(sc2Root + activeCampaignFolder + WoL.path + dir_entry.path().filename().string()), copyOptions);
            }
        }
    }
    else if(expansion == "HotS") {
        restartExpansion(HotS);
        fs::current_path(dir);
        for (const fs::directory_entry& dir_entry : fs::directory_iterator(fs::current_path())) {
            if(dir_entry.path().extension().string() == ".SC2Mod" || dir_entry.path().extension().string() == ".sc2mod") {
                fs::copy(dir_entry.path(), (fs::path)(sc2Root + "Mods/" + dir_entry.path().filename().string()), copyOptions);
            }
            else {
                fs::copy(dir_entry.path(), (fs::path)(sc2Root + activeCampaignFolder + HotS.path + dir_entry.path().filename().string()), copyOptions);
            }
        }
    }
    else if(expansion == "LotV") {
        restartExpansion(LotV);
        fs::current_path(dir);
        for (const fs::directory_entry& dir_entry : fs::directory_iterator(fs::current_path())) {
            if(dir_entry.path().filename().string() == "voidprologue") {
                fs::copy(dir_entry.path(), (fs::path)(sc2Root + activeCampaignFolder + dir_entry.path().filename().string()), copyOptions);
            }
            else if(((dir_entry.path().filename().string()).find("prologue") != std::string::npos)) {
                fs::copy(dir_entry.path(), (fs::path)(sc2Root + activeCampaignFolder + "/voidprologue/" + dir_entry.path().filename().string()), copyOptions);
            }
            else if(dir_entry.path().extension().string() == ".SC2Mod" || dir_entry.path().extension().string() == ".sc2mod") {
                fs::copy(dir_entry.path(), (fs::path)(sc2Root + "Mods/" + dir_entry.path().filename().string()), copyOptions);
            }
            else {
                fs::copy(fs::absolute(dir_entry.path()), (fs::path)(sc2Root + activeCampaignFolder + LotV.path + dir_entry.path().filename().string())), copyOptions;
            }
        }
    }
    else if(expansion == "NCO") {
        restartExpansion(NCO);
        fs::current_path(dir);
        for (const fs::directory_entry& dir_entry : fs::directory_iterator(fs::current_path())) {
            if(dir_entry.path().extension().string() == ".SC2Mod" || dir_entry.path().extension().string() == ".sc2mod") {
                fs::copy(dir_entry.path(), (fs::path)(sc2Root + "Mods/" + dir_entry.path().filename().string()), copyOptions);
            }
            else {
                fs::copy(dir_entry.path(), (fs::path)(sc2Root + activeCampaignFolder + NCO.path + dir_entry.path().filename().string()), copyOptions);
            }
        }
    }
    else {
        return false;
    }
    return true;
} 

bool CustomCampaignManager::restartExpansion(Expansion& expansion) {
    fs::current_path((fs::path)(sc2Root + activeCampaignFolder));
    if(expansion.expansion == "WoL") {
        for (const fs::directory_entry& dir_entry : fs::directory_iterator(fs::current_path())) {
            if(dir_entry.path().filename().string() != "void" && 
                dir_entry.path().filename().string() != "swarm" && 
                dir_entry.path().filename().string() != "nova" && 
                dir_entry.path().filename().string() != "voidprologue") {

                fs::remove_all(dir_entry.path());
            }
        }
        return true;
    }
    fs::remove_all((fs::path)expansion.path);
    if(expansion.expansion == "LotV") {
        fs::remove_all((fs::path)"./voidprologue");
        fs::create_directories((fs::path)"./voidprologue");
    }
    fs::create_directories((fs::path)expansion.path);

    return true;
}

bool CustomCampaignManager::restartCampaigns() {
    fs::current_path((fs::path)(sc2Root + "Maps"));
    fs::remove_all((fs::path)"./Campaign");
    fs::create_directories((fs::path)"./Campaign/swarm");
    fs::create_directories((fs::path)"./Campaign/void");
    fs::create_directories((fs::path)"./Campaign/nova");
    fs::create_directories((fs::path)"./Campaign/voidprologue");

    return true;
}

bool CustomCampaignManager::loadMetadataFile(std::string filename) {
    std::fstream file;

    file.open(filename, std::ios::in);
    if(!file.good()) {
        return false;
    }
    std::string line;
    while(std::getline(file, line)) {
        if(line.back() == '\r' || line.back() == '\n') {
            line.pop_back();
        }
        std::string key = "";
        std::string value = "";
        int pos = line.find("=");
        if(pos == std::string::npos) {
            return 2;
        }
        for(int i = 0; i < pos; i++) {
            if(line[i]!=' ') {
                std::string c(1, line[i]);
                key += c;
            }
        }
        pos++; //skip '='
        for(int i = pos; i < line.length(); i++) {
            std::string c(1, line[i]);
            value += c;
        }
        metadata[key] = value;
    }

    file.close();

    return true;
}

std::string CustomCampaignManager::getMetadataKey(std::string key) {
    if(metadata.find(key) == metadata.end()) {
        return "";
    }
    return metadata[key];
}

bool CustomCampaignManager::setSc2Root(std::string dir) {
    fs::path scPath = fs::absolute((fs::path)dir);
    if(!fs::is_directory(scPath)) {
        return false;
    }
    sc2Root = dir;
    return true;
}

void CustomCampaignManager::setCustomCampaignsFolder(std::string dir) {
    customCampaignsFolder = dir;
}

void CustomCampaignManager::setActiveCampaignFolder(std::string dir) {
    activeCampaignFolder = dir;
}



std::string CustomCampaignManager::getSc2Root() {
    return sc2Root;
}

std::string CustomCampaignManager::getCustomCampaignsFolder() {
    return customCampaignsFolder;
}

std::string CustomCampaignManager::getActiveCampaignFolder() {
    return activeCampaignFolder;
}

std::string CustomCampaignManager::getLastInstalledCampaignPath() {
    return lastInstalledCampaignPath;
}
