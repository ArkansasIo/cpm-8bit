// include/sched.hpp — redesigned kernel: cooperative round-robin scheduler
//
// Linux-inspired but OOP: Task objects inherit from kernel::Task and get
// scheduled. Tasks are cooperative (yield()) — real mode has no page tables
// and one stack, so this is the honest 16-bit analogue of CFS-lite.
#pragma once
#include "types.hpp"

namespace kernel
{
    class Task
    {
    public:
        explicit Task(const char *name, u8 priority = 5) : _name(name), _prio(priority) {}
        virtual ~Task() = default;

        const char *name() const { return _name; }
        u8 priority() const { return _prio; }
        bool finished() const { return _done; }
        u32 runs() const { return _runs; }

        // one slice of work; call yield() (return true) to be rescheduled
        virtual bool tick() = 0;

        void kill() { _done = true; }

    protected:
        bool yield() { return true; } // explicit reschedule point
        u32 _runs = 0;

    private:
        const char *_name;
        u8 _prio;
        bool _done = false;
        friend class Scheduler;
        void mark_ran() { ++_runs; }
    };

    class Scheduler
    {
    public:
        static Scheduler &instance()
        {
            static Scheduler s;
            return s;
        }

        bool add(Task &t)
        {
            if (_count >= MaxTasks)
                return false;
            _tasks[_count++] = &t;
            return true;
        }

        // run one scheduling pass: every ready task gets one tick, in
        // priority order (lower number = higher priority, stable round-robin)
        void schedule_pass()
        {
            // simple insertion sort by priority each pass — task count is tiny
            for (u8 i = 1; i < _count; ++i)
            {
                Task *key = _tasks[i];
                s8 j = s8(i - 1);
                while (j >= 0 && _tasks[j]->priority() > key->priority())
                {
                    _tasks[u8(j + 1)] = _tasks[j];
                    --j;
                }
                _tasks[u8(j + 1)] = key;
            }
            for (u8 i = 0; i < _count; ++i)
            {
                Task *t = _tasks[i];
                if (t->finished())
                    continue;
                t->mark_ran();
                const bool yielded = t->tick();
                (void)yielded;
            }
        }

        u8 count() const { return _count; }
        void reap() // remove finished tasks
        {
            u8 w = 0;
            for (u8 i = 0; i < _count; ++i)
                if (!_tasks[i]->finished())
                    _tasks[w++] = _tasks[i];
            _count = w;
        }

    private:
        static constexpr u8 MaxTasks = 8;
        Task *_tasks[MaxTasks]{};
        u8 _count = 0;
    };
} // namespace kernel
