#include "common/errno.h"
#include <common/trace.h>
#include <kernel/thread.h>
#include <string.h>
#include <type_traits>

// The following classes are used to limit access to user space
// pointers.
// UserPointer for get_thread_property (writing to user space)
// ConstUserPointer for set_thread_property (reading from user space)
// We use enable_if to check at compile time that each property
// is handled in the expected way.

struct PropID {
  constexpr static int value = TPROP_ID;
  typedef int type;
};
struct PropState {
  constexpr static int value = TPROP_STATE;
  typedef ThreadState type;
};
struct PropPermissions {
  constexpr static int value = TPROP_PERMISSIONS;
  typedef uint16_t type;
};
struct PropErrnoPtr {
  constexpr static int value = TPROP_ERRNO_PTR;
  typedef int* type;
};
struct PropChild {
  constexpr static int value = TPROP_CHILD;
  typedef int type;
};
struct PropName {
  constexpr static int value = TPROP_NAME;
  typedef char type;
};
struct PropRegisters {
  constexpr static int value = TPROP_REGISTERS;
  typedef RegisterContext type;
};
struct PropPendingSignals {
  constexpr static int value = TPROP_PENDING_SIGNALS;
  typedef uint32_t type;
};
struct PropSignalHandler {
  constexpr static int value = TPROP_SIGNAL_HANDLER;
  typedef void (*type)(uint32_t);
};

/* clang-format off */
class UserPointer {
public:
  UserPointer(void* ptr) : m_ptr(ptr) {}

  // Write a value back to userspace
  template <typename P,
      typename = typename std::enable_if<
          (std::is_same<P, PropID>::value)          ||
          (std::is_same<P, PropState>::value)       ||
          (std::is_same<P, PropPermissions>::value) ||
          (std::is_same<P, PropErrnoPtr>::value)    ||
          (std::is_same<P, PropChild>::value)
      >::type>
  void set_value(typename P::type value) {
    *static_cast<typename P::type*>(m_ptr) = value;
  }

  // Error when the parameter type is != the property type
  // Even if the compiler could implicitly cast to it.
  // E.g. p.set_value<PropChild>((uin16_t)1); is an error
  template <typename P, typename T,
      typename = typename std::enable_if<
          (std::is_same<P, PropID>::value          && !std::is_same<T, typename P::type>::value) ||
          (std::is_same<P, PropState>::value       && !std::is_same<T, typename P::type>::value) ||
          (std::is_same<P, PropPermissions>::value && !std::is_same<T, typename P::type>::value) ||
          (std::is_same<P, PropErrnoPtr>::value    && !std::is_same<T, typename P::type>::value) ||
          (std::is_same<P, PropChild>::value       && !std::is_same<T, typename P::type>::value)
      >::type>
  void set_value(T value) = delete;

  // Get the user pointer as a pointer to a specific type that you can write to
  template <typename P,
            typename = typename std::enable_if<
                (std::is_same<P, PropName>::value) ||
                (std::is_same<P, PropRegisters>::value)
            >::type>
  typename P::type* get_ptr() const {
    return static_cast<typename P::type*>(m_ptr);
  }

private:
  void* m_ptr;
};

class ConstUserPointer {
public:
  ConstUserPointer(const void* ptr) : m_ptr(ptr) {}

  // Read value out of user pointer
  template <typename P,
      typename = typename std::enable_if<
          (std::is_same<P, PropChild>::value)          ||
          (std::is_same<P, PropPendingSignals>::value) ||
          (std::is_same<P, PropSignalHandler>::value)  ||
          (std::is_same<P, PropPermissions>::value)
      >::type>
  typename P::type get_value() const {
    return *static_cast<const typename P::type*>(m_ptr);
  }

  // Get the user pointer as a const pointer you can read from
  template <typename P,
            typename = typename std::enable_if<
                (std::is_same<P, PropName>::value) ||
                (std::is_same<P, PropRegisters>::value)
            >::type>
  const typename P::type* get_ptr() const {
    return static_cast<const typename P::type*>(m_ptr);
  }

private:
  const void* m_ptr;
};
/* clang-format on */

static bool do_get_thread_property(int tid, size_t property, UserPointer res) {
  if (tid == CURRENT_THREAD) {
    tid = k_get_thread_id();
  }
  if (!is_valid_thread(tid)) {
    current_thread->err_no = E_INVALID_ID;
    return false;
  }

  Thread* thread = &all_threads[tid];
  switch (property) {
    case PropID::value:
      res.set_value<PropID>(thread->id);
      break;
    case PropName::value: {
      char* dest = res.get_ptr<PropName>();
      if (dest) {
        strncpy(dest, thread->name, THREAD_NAME_MAX_LEN);
        dest[strlen(thread->name)] = '\0';
      }
      break;
    }
    case PropChild::value:
      res.set_value<PropChild>(thread->child);
      break;
    case PropState::value:
      res.set_value<PropState>(thread->state);
      break;
    case PropPermissions::value:
      res.set_value<PropPermissions>(thread->permissions);
      break;
    case PropRegisters::value: {
      RegisterContext* ctx = res.get_ptr<PropRegisters>();
      *ctx = *(RegisterContext*)thread->stack_ptr;
      break;
    }
    case PropErrnoPtr::value:
      res.set_value<PropErrnoPtr>(&current_thread->err_no);
      break;
    default:
      current_thread->err_no = E_INVALID_ARGS;
      return false;
  }

  return true;
}

extern "C" bool k_get_thread_property(int tid, size_t property, void* res) {
  if (!res) {
    current_thread->err_no = E_INVALID_ARGS;
    return false;
  }
  return do_get_thread_property(tid, property, UserPointer(res));
}

static bool do_set_thread_property(int tid, size_t property,
                                   const ConstUserPointer value) {
  if (((tid == CURRENT_THREAD) && k_has_no_permission(TPERM_TCONFIG)) ||
      ((tid != CURRENT_THREAD) && k_has_no_permission(TPERM_TCONFIG_OTHER))) {
    current_thread->err_no = E_PERM;
    return false;
  }

  if (tid == CURRENT_THREAD) {
    tid = k_get_thread_id();
  }
  if (!is_valid_thread(tid)) {
    current_thread->err_no = E_INVALID_ID;
    return false;
  }

  Thread* thread = &all_threads[tid];
  switch (property) {
    case PropChild::value: {
      // Not sure I like one property call setting two things
      int child = value.get_value<PropChild>();
      if (is_valid_thread(child)) {
        thread->child = child;
        all_threads[child].parent = tid;
      }
      break;
    }
    case PropName::value:
      k_set_thread_name(thread, value.get_ptr<PropName>());
      break;
    case PropPermissions::value:
      thread->permissions &= ~(value.get_value<PropPermissions>());
      break;
    case PropRegisters::value: {
      // Note that setting registers on an init thread doesn't
      // serve much purpose but it won't break anything.
      const RegisterContext* ctx = value.get_ptr<PropRegisters>();
      memcpy(thread->stack_ptr, ctx, sizeof(RegisterContext));
      break;
    }
    case PropPendingSignals::value: {
      uint32_t signal = value.get_value<PropPendingSignals>();
      if (signal) {
        thread->pending_signals |= 1 << (signal - 1);
      }
      break;
    }
    case PropSignalHandler::value:
      thread->signal_handler = value.get_value<PropSignalHandler>();
      break;
    default:
      current_thread->err_no = E_INVALID_ARGS;
      return false;
  }

  return true;
}

extern "C" bool k_set_thread_property(int tid, size_t property,
                                      const void* value) {
  if (!value) {
    current_thread->err_no = E_INVALID_ARGS;
    return false;
  }
  return do_set_thread_property(tid, property, ConstUserPointer(value));
}
