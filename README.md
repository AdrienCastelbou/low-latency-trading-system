![MasterHead](https://media4.giphy.com/media/v1.Y2lkPTc5MGI3NjExenQ3azRoYXpxazR6c2xyeGZoYWRscG5wejl3cm5vdGs4b2VocDg2dCZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/VDb99mul41iLOY4vaF/giphy.gif)

# [WIP] Low Latency Trading System

This project is inspired by **Building Low Latency Applications with C++** by Sourav Ghosh. It aims to explore the concepts and techniques required to build low latency applications through the development of a complete electronic trading system.

## Project Overview

The primary objective of this project is to demonstrate efficient techniques and strategies for reducing latency in high-performance systems. The project is structured into multiple components, each representing a critical part of an electronic trading system.

### Key Components

1. **Market Data Handler**
   - Responsible for receiving and processing market data feeds in real-time.
   - Ensures that data is parsed, validated, and distributed with minimal delay.

2. **Order Management System (OMS)**
   - Manages order creation, modification, and cancellation.
   - Implements efficient data structures to handle large volumes of orders under stringent time constraints.

3. **Execution Engine**
   - Responsible for routing orders to exchanges or matching them internally.
   - Optimized for ultra-fast decision-making and trade execution.

4. **Risk Management**
   - Implements pre-trade and post-trade risk checks to ensure compliance and control.
   - Focuses on achieving low overhead without compromising the safety of the system.

5. **Performance Monitoring**
   - Provides tools to measure and benchmark system performance.
   - Includes metrics for latency, throughput, and error rates.

6. **Simulation and Backtesting**
   - Offers a controlled environment to test trading strategies and system behavior.
   - Simulates market conditions for performance evaluation and debugging.

## Highlights

- High-performance **C++** implementation optimized for low latency.
- Modular and extensible design to accommodate future improvements.
- Focus on real-world challenges faced in electronic trading systems.
