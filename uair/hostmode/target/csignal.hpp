#ifndef CSIGNAL_H__
#define CSIGNAL_H__

#include <functional>
#include <mutex>
#include <vector>
#include <unordered_map>

class CSignalBase
{
protected:
    virtual void disconnect(int index) = 0;//{ }
    friend class CSignalID;
};

class CSignalID
{
public:
    CSignalID(CSignalBase*b, int id): m_base(b), m_id(id) {}
    bool operator==(const CSignalID &other) { return m_id==other.m_id; }
    int id() const { return m_id; }
    void disconnect() { m_base->disconnect(m_id); }
private:
    CSignalBase *m_base;
    int m_id;
};

template<typename... Arg>
class CSignal: public CSignalBase
{
public:
    CSignal(): m_id(1) {}

    typedef CSignalID id_t;

    void emit(Arg... arg) {
        std::lock_guard<std::mutex> lock(m_lock);
        typename function_container_t::iterator it = m_functions.begin();
        while (it != m_functions.end()) {
            if (!it->second(arg...)) {
                it = m_functions.erase(it);
            } else {
                ++it;
            }
        }
    }

    template<typename T>
        id_t connect(T *object, bool (T::*F)(Arg... arg)) {
            std::lock_guard<std::mutex> lock(m_lock);
            unsigned id = new_id();
            m_functions[ id ] = [=](Arg... arg)->bool { return (object->*F)(arg...); };
            return CSignalID(this,id);
        }

    id_t connect( std::function<bool(Arg...)> f){
        std::lock_guard<std::mutex> lock(m_lock);
        unsigned id = new_id();
        m_functions[ id ] = f;
        return CSignalID(this,id);
    }
    bool connected() const {
        return !m_functions.empty();
    }
protected:
    virtual void disconnect(int index)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        typename function_container_t::iterator it = m_functions.find(index);
        if (it!=m_functions.end())
            m_functions.erase(it);
    }

private:
    int new_id() {
        do {
            m_id++;
        } while (m_functions.find(m_id)!=m_functions.end());
        return m_id;
    }
protected:
    typedef std::unordered_map<unsigned, std::function<bool(Arg...)> > function_container_t;
    function_container_t m_functions;
    std::mutex m_lock;
    unsigned m_id;
};

#endif
