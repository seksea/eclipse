#include <cassert>
#include <vector>

#include "funchook.h"
#include "util/log.hpp"

inline std::vector<funchook_t*> g_vecFHs;

#define HOOK(inst) o##inst.Hook(fn##inst, (void**)(&hk##inst), #inst)
#define HOOK_ASSERT(expr) assert(expr)

template <typename T>
class CHook {
  public:
	void Hook(void* pOriginalFunc, void** pHookFunc, const char* szHookName) {
		HOOK_ASSERT(pOriginalFunc && "Cannot hook at NULL");

		this->m_pFH = funchook_create( );
		this->originalFn = (decltype(this->originalFn))(pOriginalFunc);

		int rv = funchook_prepare(m_pFH, (void**)(&this->originalFn), pHookFunc);
		HOOK_ASSERT(rv == 0 && "Hook failed to prepare");

		rv = funchook_install(m_pFH, 0);
		HOOK_ASSERT(rv == 0 && "Hook failed to install");

		g_vecFHs.emplace_back(m_pFH);

		LOG("[%s] %s hooked successfully\n", "funchook", szHookName);
	}

	std::add_pointer_t<T> originalFn;
  private:
	funchook_t* m_pFH = nullptr;
};

#undef HOOK_ASSERT
