#include "../../interfaces.hpp"
#include <map>

IUIPanel* findPanel(const char* panelName) {
    IUIPanel* itr = nullptr;
    for (int i = 0; i < 4096; i++) {
        itr = panelArray->slots[i].panel;
        if (!Interfaces::panorama->AccessUIEngine()->IsValidPanelPointer(itr))
            continue;

        while (itr && Interfaces::panorama->AccessUIEngine()->IsValidPanelPointer(itr)) {
            if (strstr(itr->GetID(), panelName)) {
                return itr;
            }
            itr = itr->GetParent();
        }
    }
    return nullptr;
}

std::map<std::string_view, IUIPanel*> panelCache;
IUIPanel* IPanoramaUIEngine::getPanel(const char* panelName) {
    IUIPanel* panel = nullptr;

    if (panelCache.find(panelName) == panelCache.end() || !(panelCache.at(panelName))) {
        WARN("panorama panel %s not in cache/invalid, getting it again...", panelName);
        panel = findPanel(panelName);
        if (panel) {
            panelCache.insert(std::pair<std::string_view, IUIPanel*>(panelName, panel));
            LOG("panorama panel %s successfully got, %lx.", panelName, panel);
        }
    }
    else {
        panel = panelCache.at(panelName);
    }

    if (!panel)
        ERR("panorama error, %s panel not found.", panelName);
    
    return panel;
}