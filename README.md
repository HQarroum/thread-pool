# thread-pool
> A lock-free thread-pool implementation in C++11.

[![Build Status](https://travis-ci.org/HQarroum/thread-pool.svg?branch=master)](https://travis-ci.org/HQarroum/thread-pool)
![Production ready](https://img.shields.io/badge/status-experimental-brightgreen.svg)

Current version: **1.0.0**

Lead Maintainer: [Halim Qarroum](mailto:hqm.post@gmail.com)

## Description

This project is an implementation of a thread-pool following C++11 semantics. It aims to make it very easy to implement a producer-consumer pattern following C++11 semantics with relatively high performances, although other types of patterns can be implemented on top of this project.

<p align="center"><br>
 <img width="450" src="docs/controlled-producer-consumer.png" />
</p><br>

This project uses the lock-free [`concurrent-queue`](https://github.com/cameron314/concurrentqueue/) implementation provided by `moodycamel` as its underlying thread-safe queuing mechanism for task executions to be spread amongst different worker threads.

## Usage

To create a thread-pool instance, you simply call its constructor by providing it with the initial number of threads to provision your thread-pool instance with.

```c++
thread::pool::pool_t pool(std::thread::hardware_concurrency() + 1);
```

> Note that `std::thread::hardware_concurrency` returns the number of physical concurrent threads supported by your system.

