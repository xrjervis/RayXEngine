#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <string>
#include <functional>
#include <future>
#include <map>

class FunctionBase {
public:
	FunctionBase() = default;
	virtual ~FunctionBase() {}
};


template<typename> class NamedFunction;

template<typename R, typename... Args>
class NamedFunction<R(Args...)> : public FunctionBase{
public:
	template<typename... Ts>
	explicit NFunction(Ts&&... ts)
		: m_func(std::forward<Ts>(ts)...) {}

	R Invoke(Args&&... args) {
		return m_func(args...);
	}

private:
	std::function<R(Args...)>	m_func;
};

class NamedFunctions {
public:
	NamedFunctions() = default;
	~NamedFunctions() {
		for (auto& it : m_functions) {
			delete it.second;
		}
	}

	template<typename R, typename... Args>
	void Register(const std::string& name, NamedFunction<R(Args...)>* nf) {
		if (m_functions.find(name) == m_functions.end()) {
			FunctionBase* fBase = nf;
			m_functions[name] = fBase;
		}
		else {
			ERROR_AND_DIE("Function name already existed!");
		}
	}

	template<typename R, typename... Args>
	R Invoke(const std::string& funcName, Args&&... args) {
		if (m_functions.find(funcName) == m_functions.end()) {
			ERROR_RECOVERABLE("Function name doesn't exist");
		}
		else {
			FunctionBase* fBase = m_functions[funcName];
			//dynamic_cast doesn't work??
			NamedFunction<R(Args...)>* nf = static_cast<NamedFunction<R(Args...)>*>(fBase);
			if (nf) {
				return nf->Invoke(std::forward<Args>(args)...);
			}
		}
	}

private:
	std::map<std::string, FunctionBase*> m_functions;
};