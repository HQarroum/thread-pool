# thread-pool
> A lock-free thread-pool implementation in C++11.

[![Build Status](https://travis-ci.org/HQarroum/thread-pool.svg?branch=master)](https://travis-ci.org/HQarroum/thread-pool)
![Production ready](https://img.shields.io/badge/status-experimental.svg)

This module makes it possible to implement a producer-consumer pattern following C++11 semantics, and by using an underlying lock-free queue implementation.

Current version: **1.0.0**

Lead Maintainer: [Halim Qarroum](mailto:hqm.post@gmail.com)

## Description

This project uses the [`lock-free concurrent queue`](https://github.com/cameron314/concurrentqueue/) implementation provided by moodycamel as its underlying thread-safe queuing mechanism for task executions to be spread amongst different worker threads.

It makes it easy to implement the producer-consumer pattern in C++ with relatively high performances.

## Usage
