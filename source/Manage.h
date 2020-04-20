#pragma once
#include <Utility>

#define ManageCost(Mng) \
	public:\
		static Mng& Impl();\
	protected :\
	Mng() {}\
	class Cost { ~Cost() { delete Mng::theInstance; } };\
	static Mng* theInstance;\

#define ManageImplement(Mng) \
	Mng* Mng::theInstance = nullptr;\
	Mng& Mng::Impl()\
	{\
		if (Mng::theInstance == nullptr)\
		{\
			Mng::theInstance = new Mng();\
		}\
		return *Mng::theInstance;\
	}\

//class ManageImpl
//{
//protected:
//	ManageImpl() {}
//};
//
//template <class Impl >
//class Manage : Impl
//{
//public:
//	static Manage<Impl>& Get()
//	{
//		if (Manage<Impl>::theInstance == nullptr)
//		{
//			theInstance = new Manage<Impl>();
//		}
//		return *theInstance;
//	}
//
//protected:
//	class Cost { ~Cost() { delete Manage<Impl>::theInstance; } };
//	static Cost cst;
//
//	static Manage<Impl>* theInstance;
//
//	template <class... Args>
//	Manage(Args &&... args) : Impl(std::forward<Args>(args)...) {}
//};

//#define ImplementManager(Impl) \
//template class Manage<Impl>; \
//Manage<Impl>& Manage ## Impl()\
//{\
//	return Manage<Impl>::Get();\
//}\