#!/bin/bash
ip link set lo up
ip addr add 127.0.99.101/8 dev lo