/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * signal.h - Signal & slot implementation
 */
#ifndef __LIBCAMERA_SIGNAL_H__
#define __LIBCAMERA_SIGNAL_H__

#include <list>
#include <type_traits>
#include <vector>

#include <libcamera/bound_method.h>
#include <libcamera/object.h>

namespace libcamera {

class SignalBase
{
public:
	template<typename T>
	void disconnect(T *obj)
	{
		for (auto iter = slots_.begin(); iter != slots_.end(); ) {
			BoundMethodBase *slot = *iter;
			if (slot->match(obj)) {
				iter = slots_.erase(iter);
				delete slot;
			} else {
				++iter;
			}
		}
	}

protected:
	friend class Object;
	std::list<BoundMethodBase *> slots_;
};

template<typename... Args>
class Signal : public SignalBase
{
public:
	Signal() {}
	~Signal()
	{
		for (BoundMethodBase *slot : slots_) {
			Object *object = slot->object();
			if (object)
				object->disconnect(this);
			delete slot;
		}
	}

#ifndef __DOXYGEN__
	template<typename T, typename R, typename std::enable_if<std::is_base_of<Object, T>::value>::type * = nullptr>
	void connect(T *obj, R (T::*func)(Args...),
		     ConnectionType type = ConnectionTypeAuto)
	{
		Object *object = static_cast<Object *>(obj);
		object->connect(this);
		slots_.push_back(new BoundMethodMember<T, void, Args...>(obj, object, func, type));
	}

	template<typename T, typename R, typename std::enable_if<!std::is_base_of<Object, T>::value>::type * = nullptr>
#else
	template<typename T, typename R>
#endif
	void connect(T *obj, R (T::*func)(Args...))
	{
		slots_.push_back(new BoundMethodMember<T, R, Args...>(obj, nullptr, func));
	}

	template<typename R>
	void connect(R (*func)(Args...))
	{
		slots_.push_back(new BoundMethodStatic<R, Args...>(func));
	}

	void disconnect()
	{
		for (BoundMethodBase *slot : slots_)
			delete slot;
		slots_.clear();
	}

	template<typename T>
	void disconnect(T *obj)
	{
		SignalBase::disconnect(obj);
	}

	template<typename T, typename R>
	void disconnect(T *obj, R (T::*func)(Args...))
	{
		for (auto iter = slots_.begin(); iter != slots_.end(); ) {
			BoundMethodArgs<R, Args...> *slot =
				static_cast<BoundMethodArgs<R, Args...> *>(*iter);
			/*
			 * If the object matches the slot, the slot is
			 * guaranteed to be a member slot, so we can safely
			 * cast it to BoundMethodMember<T, Args...> to match
			 * func.
			 */
			if (slot->match(obj) &&
			    static_cast<BoundMethodMember<T, R, Args...> *>(slot)->match(func)) {
				iter = slots_.erase(iter);
				delete slot;
			} else {
				++iter;
			}
		}
	}

	template<typename R>
	void disconnect(R (*func)(Args...))
	{
		for (auto iter = slots_.begin(); iter != slots_.end(); ) {
			BoundMethodArgs<R, Args...> *slot = *iter;
			if (slot->match(nullptr) &&
			    static_cast<BoundMethodStatic<R, Args...> *>(slot)->match(func)) {
				iter = slots_.erase(iter);
				delete slot;
			} else {
				++iter;
			}
		}
	}

	void emit(Args... args)
	{
		/*
		 * Make a copy of the slots list as the slot could call the
		 * disconnect operation, invalidating the iterator.
		 */
		std::vector<BoundMethodBase *> slots{ slots_.begin(), slots_.end() };
		for (BoundMethodBase *slot : slots)
			static_cast<BoundMethodArgs<void, Args...> *>(slot)->activate(args...);
	}
};

} /* namespace libcamera */

#endif /* __LIBCAMERA_SIGNAL_H__ */
