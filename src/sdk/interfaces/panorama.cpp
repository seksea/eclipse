#include "../../interfaces.hpp"

/* Get panorama root panel (credit LWSS' McDota project) */
IUIPanel* IPanoramaUIEngine::getRoot() {
    IUIPanel* itr = nullptr;
    for (int i = 0; i < 4096; i++) {
        itr = panelArray->slots[i].panel;
        if (!Interfaces::panorama->AccessUIEngine()->IsValidPanelPointer(itr))
            continue;

        while (itr && Interfaces::panorama->AccessUIEngine()->IsValidPanelPointer(itr)) {
            LOG("%s", itr->GetID());
            if (!strcmp(itr->GetID(), "CSGOMainMenu")) {
                return itr;
            }
            itr = itr->GetParent();
        }
    }
    return nullptr;
}