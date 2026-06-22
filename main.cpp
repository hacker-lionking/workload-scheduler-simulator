#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;

class Job {
public:
    int id, burstTime, remainingTime, priority, waitTime = 0;
    bool isCompleted = false;
    bool isRunning = false;
    int assignedWorker = -1;

    Job(int id, int burstTime, int priority = 5)
        : id(id), burstTime(burstTime), remainingTime(burstTime), priority(priority) {}
};

class Worker {
public:
    int id, capacity;
    int currentJobId = -1;
    bool isBusy = false;

    Worker(int id, int capacity) : id(id), capacity(capacity) {}

    bool canTakeNewJob() {
        return !isBusy && capacity > 0;
    }

    void assignJob(Job* job) {
        currentJobId = job->id;
        isBusy = true;
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

class ShortestJobFirst : public SchedulingStrategy {
public:
    string getStrategyName() override { return "Shortest-Job-First"; }

    vector<Job*> pickJobs(vector<Job>& jobs, vector<Worker>& workers) override {
        vector<Job*> pending;
        for (auto& j : jobs) {
            if (!j.isCompleted && !j.isRunning && j.remainingTime > 0) {
                pending.push_back(&j);
            }
        }
        if (pending.empty()) return {};

        sort(pending.begin(), pending.end(), [](Job* a, Job* b) {
            return a->remainingTime < b->remainingTime;
        });

        vector<Job*> result;
        vector<bool> picked(pending.size(), false);

        for (auto& w : workers) {
            if (!w.canTakeNewJob()) continue;
            for (size_t i = 0; i < pending.size(); i++) {
                if (!picked[i]) {
                    result.push_back(pending[i]);
                    picked[i] = true;
                    break;
                }
            }
        }
        return result;
    }
};

class PriorityAging : public SchedulingStrategy {
private:
    int agingDivisor;
public:
    PriorityAging(int divisor = 3) : agingDivisor(max(1, divisor)) {}

    string getStrategyName() override { return "Priority-Aging"; }

    vector<Job*> pickJobs(vector<Job>& jobs, vector<Worker>& workers) override {
        vector<Job*> pending;
        for (auto& j : jobs) {
            if (!j.isCompleted && !j.isRunning && j.remainingTime > 0) {
                pending.push_back(&j);
            }
        }
        if (pending.empty()) return {};

        sort(pending.begin(), pending.end(), [this](Job* a, Job* b) {
            int ea = a->priority - (a->waitTime / agingDivisor);
            int eb = b->priority - (b->waitTime / agingDivisor);
            return ea < eb;
        });

        vector<Job*> result;
        vector<bool> picked(pending.size(), false);

        for (auto& w : workers) {
            if (!w.canTakeNewJob()) continue;
            for (size_t i = 0; i < pending.size(); i++) {
                if (!picked[i]) {
                    result.push_back(pending[i]);
                    picked[i] = true;
                    break;
                }
            }
        }
        return result;
    }
};

class KnapsackDP : public SchedulingStrategy {
private:
    static const int MAX_CAP = 10000;
public:
    string getStrategyName() override { return "Knapsack-DP"; }

    vector<Job*> pickJobs(vector<Job>& jobs, vector<Worker>& workers) override {
        vector<Job*> pending;
        for (auto& j : jobs) {
            if (!j.isCompleted && !j.isRunning && j.remainingTime > 0) {
                pending.push_back(&j);
            }
        }
        if (pending.empty()) return {};

        int totalCap = 0;
        int maxWorkerCap = 0;
        for (auto& w : workers) {
            if (w.canTakeNewJob()) {
                totalCap += w.capacity;
                maxWorkerCap = max(maxWorkerCap, w.capacity);
            }
        }

        if (totalCap <= 0 || maxWorkerCap <= 0) return {};

        // ---- CAP BOTH TO MAX_CAP ----
        totalCap = min(totalCap, MAX_CAP);
        maxWorkerCap = min(maxWorkerCap, totalCap);  // <-- FIX: now consistent

        int n = pending.size();
        vector<int> val(n), wt(n);
        for (int i = 0; i < n; i++) {
            val[i] = max(1, 10 - pending[i]->priority);
            wt[i] = min(pending[i]->remainingTime, maxWorkerCap);
        }

        vector<int> dp(totalCap + 1, 0);
        vector<vector<bool>> keep(n, vector<bool>(totalCap + 1, false));

        for (int i = 0; i < n; i++) {
            for (int c = totalCap; c >= wt[i]; c--) {
                int newVal = dp[c - wt[i]] + val[i];
                if (newVal > dp[c]) {
                    dp[c] = newVal;
                    keep[i][c] = true;
                }
            }
        }

        vector<Job*> chosen;
        int rem = totalCap;
        for (int i = n - 1; i >= 0; i--) {
            if (keep[i][rem]) {
                chosen.push_back(pending[i]);
                rem -= wt[i];
            }
        }

        return chosen;
    }
};

class SchedulerEngine {
private:
    vector<Job> jobs;
    vector<Worker> workers;
    SchedulingStrategy* strategy;

public:
    SchedulerEngine(const vector<Job>& jobList, const vector<Worker>& workerList,
                    SchedulingStrategy* strat)
        : jobs(jobList), workers(workerList), strategy(strat) {}

    bool allJobsDone() {
        for (auto& j : jobs) {
            if (!j.isCompleted) return false;
        }
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
        }

        int tick = 0;
        string prevRunningMsg = "";
        bool skipped = false;

        while (!allJobsDone() && tick < maxTicks) {
            string doneMsg = "";
            string assignMsg = "";
            string runningMsg = "";

            for (auto& w : workers) {
                if (w.isBusy) {
                    int jobId = w.currentJobId;
                    w.doWork(jobs);
                    if (!w.isBusy) {
                        for (auto& j : jobs) {
                            if (j.id == jobId && j.remainingTime <= 0) {
                                j.isCompleted = true;
                                j.isRunning = false;
                                doneMsg += "J" + to_string(j.id) + " DONE! ";
                                break;
                            }
                        }
                    }
                }
            }

            for (auto& j : jobs) {
                if (!j.isCompleted && !j.isRunning) {
                    j.waitTime++;
                }
            }

            vector<Job*> picks = strategy->pickJobs(jobs, workers);

            int pickIdx = 0;
            for (auto& w : workers) {
                if (w.canTakeNewJob() && pickIdx < (int)picks.size()) {
                    Job* best = picks[pickIdx];
                    w.assignJob(best);
                    best->isRunning = true;
                    best->assignedWorker = w.id;
                    assignMsg += "J" + to_string(best->id) + "->W" + to_string(w.id) + " ";
                    pickIdx++;
                }
            }

            for (auto& w : workers) {
                if (w.isBusy) {
                    runningMsg += "J" + to_string(w.currentJobId) + "->W" + to_string(w.id) + " ";
                }
            }

            if (verbose) {
                bool hasChange = !assignMsg.empty() || !doneMsg.empty() || runningMsg != prevRunningMsg;

                if (hasChange) {
                    if (skipped) {
                        cout << "...\n";
                        skipped = false;
                    }
                    cout << "Tick " << tick << " -> ";
                    if (!assignMsg.empty()) cout << assignMsg;
                    if (!doneMsg.empty()) cout << doneMsg;
                    if (!runningMsg.empty() && assignMsg.empty() && doneMsg.empty()) cout << runningMsg;
                    if (assignMsg.empty() && doneMsg.empty() && runningMsg.empty()) cout << "Idle";
                    cout << "\n";
                } else {
                    skipped = true;
                }

                prevRunningMsg = runningMsg;
            }

            tick++;
        }

        if (verbose) {
            cout << "Ticks: " << tick << "\n\n";
        }

        return tick;
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int n, m;
    if (!(cin >> n)) return 1;

    vector<Job> baseJobs;
    for (int i = 0; i < n; i++) {
        int id, b, p;
        cin >> id >> b >> p;
        baseJobs.emplace_back(id, b, p);
    }

    cin >> m;
    vector<Worker> baseWorkers;
    for (int i = 0; i < m; i++) {
        int id, c;
        cin >> id >> c;
        baseWorkers.emplace_back(id, c);
    }

    ShortestJobFirst sjf;
    SchedulerEngine e1(baseJobs, baseWorkers, &sjf);
    int t1 = e1.runSimulation(10000, false);

    PriorityAging aging(2);
    SchedulerEngine e2(baseJobs, baseWorkers, &aging);
    int t2 = e2.runSimulation(10000, false);

    KnapsackDP dp;
    SchedulerEngine e3(baseJobs, baseWorkers, &dp);
    int t3 = e3.runSimulation(10000, false);

    cout << "=== COMPARISON ===\n";
    cout << "SJF  : " << t1 << " ticks\n";
    cout << "Aging: " << t2 << " ticks\n";
    cout << "DP   : " << t3 << " ticks\n\n";

    cout << "=== Shortest-Job-First ===\n";
    e1.runSimulation(10000, true);

    cout << "=== Priority-Aging ===\n";
    e2.runSimulation(10000, true);

    cout << "=== Knapsack-DP ===\n";
    e3.runSimulation(10000, true);

    return 0;
}
