#include "common/errno.h"
#include <common/assert.h>
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

/* clang-format off */
class UserPointer {
public:
  UserPointer(void* ptr) : m_ptr(ptr) {}

  template <
      unsigned Property, typename T,
      typename = typename std::enable_if<
          (Property == TPROP_ID          && std::is_same<T, int>::value)         ||
          (Property == TPROP_STATE       && std::is_same<T, ThreadState>::value) ||
          (Property == TPROP_PERMISSIONS && std::is_same<T, uint16_t>::value)    ||
          (Property == TPROP_ERRNO_PTR   && std::is_same<T, int*>::value)        ||
          (Property == TPROP_CHILD       && std::is_same<T, int>::value)>::type>
  void set_value(T value) {
    *static_cast<T*>(m_ptr) = value;
  }

  template <unsigned Property, typename T,
            typename = typename std::enable_if<
                (Property == TPROP_NAME      && std::is_same<T, char>::value) ||
                (Property == TPROP_REGISTERS && std::is_same<T, RegisterContext>::value)>::type>
  T* get_ptr() const {
    return static_cast<T*>(m_ptr);
  }

private:
  void* m_ptr;
};

class ConstUserPointer {
public:
  ConstUserPointer(const void* ptr) : m_ptr(ptr) {}

  template <
      unsigned Property, typename T,
      typename = typename std::enable_if<
          (Property == TPROP_NAME            && std::is_same<T, const char*>::value)        ||
          (Property == TPROP_CHILD           && std::is_same<T, int>::value)                ||
          (Property == TPROP_PENDING_SIGNALS && std::is_same<T, uint32_t>::value)           ||
          (Property == TPROP_SIGNAL_HANDLER  && std::is_same<T, void (*)(uint32_t)>::value) ||
          (Property == TPROP_PERMISSIONS     && std::is_same<T, uint16_t>::value)>::type>
  T get_value() const {
    return *static_cast<const T*>(m_ptr);
  }

  template <unsigned Property, typename T,
            typename = typename std::enable_if<
                Property == TPROP_REGISTERS && std::is_same<T, RegisterContext>::value>::type>
  const T* get_ptr() const {
    return static_cast<const T*>(m_ptr);
  }

private:
  const void* m_ptr;
};
/* clang-format on */

static bool do_get_thread_property(int tid, size_t property, UserPointer& res) {
  if (tid == CURRENT_THREAD) {
    tid = k_get_thread_id();
  }
  if (!is_valid_thread(tid)) {
    current_thread->err_no = E_INVALID_ID;
    return false;
  }

  Thread* thread = &all_threads[tid];
  switch (property) {
    case TPROP_ID:
      res.set_value<TPROP_ID, int>(thread->id);
      break;
    case TPROP_NAME: {
      char* dest = res.get_ptr<TPROP_NAME, char>();
      if (dest) {
        strncpy(dest, thread->name, THREAD_NAME_MAX_LEN);
        dest[strlen(thread->name)] = '\0';
      }
      break;
    }
    case TPROP_CHILD:
      res.set_value<TPROP_CHILD, int>(thread->child);
      break;
    case TPROP_STATE:
      res.set_value<TPROP_STATE, ThreadState>(thread->state);
      break;
    case TPROP_PERMISSIONS:
      res.set_value<TPROP_PERMISSIONS, uint16_t>(thread->permissions);
      break;
    case TPROP_REGISTERS: {
      RegisterContext* ctx = res.get_ptr<TPROP_REGISTERS, RegisterContext>();
      *ctx = *(RegisterContext*)thread->stack_ptr;
      break;
    }
    case TPROP_ERRNO_PTR:
      res.set_value<TPROP_ERRNO_PTR, int*>(&current_thread->err_no);
      break;
    default:
      // TODO: E_INVALID_ARGS for this and res ptr
      assert(0);
  }

  return true;
}

extern "C" bool k_get_thread_property(int tid, size_t property, void* res) {
  UserPointer p(res);
  return do_get_thread_property(tid, property, p);
}

static bool do_set_thread_property(int tid, size_t property,
                                   const ConstUserPointer& value) {
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
    case TPROP_CHILD: {
      // Not sure I like one property call setting two things
      int child = value.get_value<TPROP_CHILD, int>();
      if (is_valid_thread(child)) {
        thread->child = child;
        all_threads[child].parent = tid;
      }
      break;
    }
    case TPROP_NAME:
      k_set_thread_name(thread, value.get_value<TPROP_NAME, const char*>());
      break;
    case TPROP_PERMISSIONS:
      thread->permissions &= ~(value.get_value<TPROP_PERMISSIONS, uint16_t>());
      break;
    case TPROP_REGISTERS: {
      // Note that setting registers on an init thread doesn't
      // serve much purpose but it won't break anything.
      const RegisterContext* ctx =
          value.get_ptr<TPROP_REGISTERS, RegisterContext>();
      memcpy(thread->stack_ptr, ctx, sizeof(RegisterContext));
      break;
    }
    case TPROP_PENDING_SIGNALS: {
      uint32_t signal = value.get_value<TPROP_PENDING_SIGNALS, uint32_t>();
      if (signal) {
        thread->pending_signals |= 1 << (signal - 1);
      }
      break;
    }
    case TPROP_SIGNAL_HANDLER:
      thread->signal_handler =
          value.get_value<TPROP_SIGNAL_HANDLER, void (*)(uint32_t)>();
      break;
    default:
      // TODO: E_INVALID_ARGS for this and value ptr
      assert(0);
  }

  return true;
}

extern "C" bool k_set_thread_property(int tid, size_t property,
                                      const void* value) {
  ConstUserPointer p(value);
  return do_set_thread_property(tid, property, value);
}
