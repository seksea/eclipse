#include "netvars.hpp"

namespace Netvars {
	void dumpOffsetsFromTable(RecvTable* table, int additionalOffset = 0) {
		for (int i = 0; i < table->m_nProps; i++) {
			RecvProp* prop = &table->m_pProps[i];

			// Hack to ignore the weird netvars that are just counting up in hexadecimal by seeing if the first character is a
			// number, or its length is 1 (for example a,b,c,d,e,f (no netvars get further than 9f in hex)). We also
			// ignore ones named "baseclass".
			if (strcmp("baseclass", prop->m_pVarName) == 0 || isdigit(prop->m_pVarName[0]) || strlen(prop->m_pVarName) == 1)
				continue;

			// If the netvar contains another table, recursively dump from that table and ensure we add the offset of the
			// table to the offset of any props pulled from it.
			if (prop->m_pDataTable) {
				dumpOffsetsFromTable(prop->m_pDataTable, additionalOffset + prop->m_Offset);
				netvars[std::pair<std::string_view, std::string_view>(table->m_pNetTableName, prop->m_pVarName)] = 
					std::pair<RecvProp*, uintptr_t>(prop, additionalOffset + prop->m_Offset);
			}
			else {
				// The netvar isn't another table, dump to Netvars::netvarMap
				netvars[std::pair<std::string_view, std::string_view>(table->m_pNetTableName, prop->m_pVarName)] = 
					std::pair<RecvProp*, uintptr_t>(prop, additionalOffset + prop->m_Offset);
			}
		}
	}

    void init() {
        LOG("Initialising Netvars...");
        for (ClientClass* cur = Interfaces::client->getAllClasses(); cur; cur = cur->m_pNext) {
            dumpOffsetsFromTable(cur->m_pRecvTable);
        }
		FILE* f = fopen("netvars.txt", "w");
		for (auto& nvar: netvars) {
			char buf[1024];
            snprintf(buf, sizeof(buf), "	NETVAR(\"%s\", \"%s\", n%s__%s, %s) \\\n", 
				nvar.first.first.data(), nvar.first.second.data(),
				nvar.first.first.data(), nvar.first.second.data(),
				nvar.second.first->m_RecvType == DPT_Int ? "int" :
				nvar.second.first->m_RecvType == DPT_Float ? "float" : 
				nvar.second.first->m_RecvType == DPT_Vector ? "Vector" :
				nvar.second.first->m_RecvType == DPT_VectorXY ? "QAngle" : "void*");
			fputs(buf, f);
        }
		fclose(f);
		LOG("Initialised Netvars.");
    }
}