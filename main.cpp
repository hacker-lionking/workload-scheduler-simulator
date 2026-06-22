#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>

using namespace std;

class Job {
public:
    int id, burstTime, remainingTime, priority, waitTime = 0;
    bool isCompleted = false;
    bool isRunning = false;
    int assignedWorker = -1;

    Job(int id, int burstTime, int priority = 5)
        : id(id), burstTime(max(0, burstTime)), remainingTime(max(0, burstTime)), priority(priority) {}
};

class Worker {
public:
    int id, capacity;
    int currentJobId = -1;
    bool isBusy = false;
    double assignedLoad = 0.0;

    Worker(int id, int capacity) : id(id), capacity(max(0, capacity)) {}

    bool canTakeNewJob() { return !isBusy && capacity > 0; }

    void assignJob(Job* job) {
        currentJobId = job->id;
        isBusy = true;
        assignedLoad += ceil((double)job->burstTime / capacity);
    }

    void doWork(vector<Job>& jobs) {
        if (!isBusy) return;
        for (auto& j : jobs) {
            if (j.id == currentJobId && !j.isCompleted) {
                j.remainingTime -= capacity;
                if (j.remainingTime <= 0) {
                    j.isCompleted = true;
                    j.isRunning = false;
                    isBusy = false;
                    currentJobId = -1;
                }
                break;
            }
        }
    }
};

class SchedulingStrategy {
public:
    virtual vector<Job*> pickJobs(vector<Job>& jobs, vector<Worker>& workers) = 0;
    virtual string getStrategyName() = 0;
    virtual ~SchedulingStrategy() = default;
};

class FCFS : public SchedulingStrategy {
public:
    string getStrategyName() override { return "FCFS"; }
    vector<Job*> pickJobs(vector<Job>& jobs, vector<Worker>& workers) override {
        vector<Job*> pending;
        for (auto& j : jobs) if (!j.isCompleted && !j.isRunning && j.remainingTime > 0) pending.push_back(&j);
        if (pending.empty()) return {};

        sort(pending.begin(), pending.end(), [](Job* a, Job* b) { return a->id < b->id; });

        vector<Job*> result;
        vector<bool> picked(pending.size(), false);
        for (auto& w : workers) {
            if (!w.canTakeNewJob()) continue;
            for (size_t i = 0; i < pending.size(); i++) {
                if (!picked[i]) { result.push_back(pending[i]); picked[i] = true; break; }
            }
        }
        return result;
    }
};

class SJF : public SchedulingStrategy {
public:
    string getStrategyName() override { return "SJF"; }
    vector<Job*> pickJobs(vector<Job>& jobs, vector<Worker>& workers) override {
        vector<Job*> pending;
        for (auto& j : jobs) if (!j.isCompleted && !j.isRunning && j.remainingTime > 0) pending.push_back(&j);
        if (pending.empty()) return {};

        sort(pending.begin(), pending.end(), [](Job* a, Job* b) {
            if (a->remainingTime != b->remainingTime) return a->remainingTime < b->remainingTime;
            return a->id < b->id;
        });

        vector<Job*> result;
        vector<bool> picked(pending.size(), false);
        for (auto& w : workers) {
            if (!w.canTakeNewJob()) continue;
            for (size_t i = 0; i < pending.size(); i++) {
                if (!picked[i]) { result.push_back(pending[i]); picked[i] = true; break; }
            }
        }
        return result;
    }
};

class PriorityAging : public SchedulingStrategy {
    int agingDivisor;
public:
    PriorityAging(int d = 3) : agingDivisor(max(1, d)) {}
    string getStrategyName() override { return "Priority-Aging"; }

    vector<Job*> pickJobs(vector<Job>& jobs, vector<Worker>& workers) override {
        vector<Job*> pending;
        for (auto& j : jobs) if (!j.isCompleted && !j.isRunning && j.remainingTime > 0) pending.push_back(&j);
        if (pending.empty()) return {};

        sort(pending.begin(), pending.end(), [this](Job* a, Job* b) {
            int ea = a->priority - (a->waitTime / agingDivisor);
            int eb = b->priority - (b->waitTime / agingDivisor);
            if (ea != eb) return ea < eb;
            return a->id < b->id;
        });

        vector<Job*> result;
        vector<bool> picked(pending.size(), false);
        for (auto& w : workers) {
            if (!w.canTakeNewJob()) continue;
            for (size_t i = 0; i < pending.size(); i++) {
                if (!picked[i]) { result.push_back(pending[i]); picked[i] = true; break; }
            }
        }
        return result;
    }
};

class HRRN : public SchedulingStrategy {
public:
    string getStrategyName() override { return "HRRN"; }

    vector<Job*> pickJobs(vector<Job>& jobs, vector<Worker>& workers) override {
        vector<Job*> pending;
        for (auto& j : jobs) if (!j.isCompleted && !j.isRunning && j.remainingTime > 0) pending.push_back(&j);
        if (pending.empty()) return {};

        sort(pending.begin(), pending.end(), [](Job* a, Job* b) {
            double rrA = (double)(a->waitTime + a->burstTime) / max(1, a->burstTime);
            double rrB = (double)(b->waitTime + b->burstTime) / max(1, b->burstTime);
            if (rrA != rrB) return rrA > rrB;
            return a->id < b->id; 
        });

        vector<Job*> result;
        vector<bool> picked(pending.size(), false);
        for (auto& w : workers) {
            if (!w.canTakeNewJob()) continue;
            for (size_t i = 0; i < pending.size(); i++) {
                if (!picked[i]) { result.push_back(pending[i]); picked[i] = true; break; }
            }
        }
        return result;
    }
};

class LoadBalancing : public SchedulingStrategy {
public:
    string getStrategyName() override { return "Load-Balancing"; }

    vector<Job*> pickJobs(vector<Job>& jobs, vector<Worker>& workers) override {
        vector<Job*> pending;
        for (auto& j : jobs) if (!j.isCompleted && !j.isRunning && j.remainingTime > 0) pending.push_back(&j);
        if (pending.empty()) return {};

       
        sort(pending.begin(), pending.end(), [](Job* a, Job* b) {
            if (a->remainingTime != b->remainingTime) return a->remainingTime > b->remainingTime;
            return a->id < b->id;
        });

        vector<Job*> result;
        int freeCount = 0;
        for (auto& w : workers) if (w.canTakeNewJob()) freeCount++;

        for (int i = 0; i < freeCount && i < (int)pending.size(); i++) {
            result.push_back(pending[i]);
        }
        return result;
    }
};

class SchedulerEngine {
    vector<Job> jobs;
    vector<Worker> workers;
    SchedulingStrategy* strategy;

public:
    SchedulerEngine(const vector<Job>& jl, const vector<Worker>& wl, SchedulingStrategy* s)
        : jobs(jl), workers(wl), strategy(s) {}

    bool allJobsDone() {
        for (auto& j : jobs) if (!j.isCompleted) return false;
        return true;
    }

    int runSimulation(int maxTicks = 10000, bool verbose = true) {
        for (auto& j : jobs) {
            j.remainingTime = j.burstTime;
            j.isCompleted = (j.burstTime <= 0);
            j.isRunning = false;
            j.waitTime = 0;
            j.assignedWorker = -1;
        }
        for (auto& w : workers) {
            w.isBusy = false;
            w.currentJobId = -1;
            w.assignedLoad = 0.0;
        }

        int tick = 0;
        string prevRun = "";
        bool skipped = false;

        while (!allJobsDone() && tick < maxTicks) {
            string doneMsg, assignMsg, runMsg;

            for (auto& w : workers) {
                if (w.isBusy) {
                    int jid = w.currentJobId;
                    w.doWork(jobs);
                    if (!w.isBusy) {
                        for (auto& j : jobs) {
                            if (j.id == jid && j.remainingTime <= 0) {
                                j.isCompleted = true;
                                j.isRunning = false;
                                doneMsg += "J" + to_string(j.id) + " DONE! ";
                                break;
                            }
                        }
                    }
                }
            }

            for (auto& j : jobs) if (!j.isCompleted && !j.isRunning) j.waitTime++;

            auto picks = strategy->pickJobs(jobs, workers);

            if (strategy->getStrategyName() == "Load-Balancing") {
                vector<Worker*> freeWorkers;
                for (auto& w : workers) if (w.canTakeNewJob()) freeWorkers.push_back(&w);

               
                sort(freeWorkers.begin(), freeWorkers.end(), [](Worker* a, Worker* b) {
                    if (a->assignedLoad != b->assignedLoad) return a->assignedLoad < b->assignedLoad;
                    return a->capacity > b->capacity; 
                });

                size_t pi = 0;
                for (auto* w : freeWorkers) {
                    if (pi < picks.size()) {
                        w->assignJob(picks[pi]);
                        picks[pi]->isRunning = true;
                        picks[pi]->assignedWorker = w->id;
                        assignMsg += "J" + to_string(picks[pi]->id) + "->W" + to_string(w->id) + " ";
                        pi++;
                    }
                }
            } else {
                size_t pi = 0;
                for (auto& w : workers) {
                    if (w.canTakeNewJob() && pi < picks.size()) {
                        w.assignJob(picks[pi]);
                        picks[pi]->isRunning = true;
                        picks[pi]->assignedWorker = w.id;
                        assignMsg += "J" + to_string(picks[pi]->id) + "->W" + to_string(w.id) + " ";
                        pi++;
                    }
                }
            }

            for (auto& w : workers) if (w.isBusy) runMsg += "J" + to_string(w.currentJobId) + "->W" + to_string(w.id) + " ";

            if (verbose) {
                bool changed = !assignMsg.empty() || !doneMsg.empty() || runMsg != prevRun;
                if (changed) {
                    if (skipped) { cout << "...\n"; skipped = false; }
                    cout << "Tick " << tick << " -> ";
                    if (!assignMsg.empty()) cout << assignMsg;
                    if (!doneMsg.empty()) cout << doneMsg;
                    if (runMsg != "" && assignMsg.empty() && doneMsg.empty()) cout << runMsg;
                    if (assignMsg.empty() && doneMsg.empty() && runMsg.empty()) cout << "Idle";
                    cout << "\n";
                } else { skipped = true; }
                prevRun = runMsg;
            }
            tick++;
        }
        if (verbose) cout << "Ticks: " << tick << "\n\n";
        return tick;
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int n, m;
    if (!(cin >> n)) return 1;

    vector<Job> jobs;
    for (int i = 0; i < n; i++) {
        int id, b, p; cin >> id >> b >> p;
        jobs.emplace_back(id, b, p);
    }

    cin >> m;
    vector<Worker> workers;
    for (int i = 0; i < m; i++) {
        int id, c; cin >> id >> c;
        workers.emplace_back(id, c);
    }

    FCFS fcfs; SJF sjf; PriorityAging aging(2); HRRN hrrn; LoadBalancing lb;

    SchedulerEngine e1(jobs, workers, &fcfs); int t1 = e1.runSimulation(10000, false);
    SchedulerEngine e2(jobs, workers, &sjf); int t2 = e2.runSimulation(10000, false);
    SchedulerEngine e3(jobs, workers, &aging); int t3 = e3.runSimulation(10000, false);
    SchedulerEngine e4(jobs, workers, &hrrn); int t4 = e4.runSimulation(10000, false);
    SchedulerEngine e5(jobs, workers, &lb); int t5 = e5.runSimulation(10000, false);

    cout << "=== COMPARISON ===\n";
    cout << "FCFS        : " << t1 << " ticks\n";
    cout << "SJF         : " << t2 << " ticks\n";
    cout << "Aging       : " << t3 << " ticks\n";
    cout << "HRRN        : " << t4 << " ticks\n";
    cout << "Load-Balance: " << t5 << " ticks\n\n";

    cout << "=== FCFS ===\n"; e1.runSimulation(10000, true);
    cout << "=== SJF ===\n"; e2.runSimulation(10000, true);
    cout << "=== Priority-Aging ===\n"; e3.runSimulation(10000, true);
    cout << "=== HRRN ===\n"; e4.runSimulation(10000, true);
    cout << "=== Load-Balancing ===\n"; e5.runSimulation(10000, true);

    return 0;
}
