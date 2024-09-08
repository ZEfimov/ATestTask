#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <gtest\gtest.h>
#include "OrderCache.h"

using namespace std::chrono_literals;

// Global flag to indicate test failure
std::atomic<bool> test_failed{false};

// Custom GTest event listener to set the flag on failure
class FailureListener : public ::testing::EmptyTestEventListener {
    void OnTestPartResult(const ::testing::TestPartResult& result) override {
        if (result.failed()) {
            test_failed = true;
        }
    }
};

// Macro to check the global failure flag
#define CHECK_GLOBAL_FAILURE_FLAG() \
    if (test_failed) { \
        GTEST_SKIP() << "\033[38;5;214mTest skipped due to a previous failure.\033[0m"; \
        return; \
    }
// #define CHECK_GLOBAL_FAILURE_FLAG() if (test_failed) return;

class OrderCacheTest : public ::testing::Test {
protected:
    OrderCache cache;
    std::vector<std::string> users;
    std::vector<std::string> secIds;
    std::vector<std::string> companies;
    std::vector<std::string> sides{"Buy", "Sell"};
    std::mt19937 gen;
    static double benchmark_time;  // Static variable to store the benchmark time

    // Constants for the test
    static constexpr unsigned int NUM_USERS = 1000;
    static constexpr unsigned int NUM_COMPANIES = 100;
    static constexpr unsigned int NUM_SECURITIES = 1000;
    static constexpr unsigned int ORDER_MIN_QTY = 100;
    static constexpr unsigned int ORDER_MAX_QTY = 5000;
    static constexpr unsigned int ORDER_QTY_MULTIPLIER = 100;

    // ANSI escape code for blue text
    const char* BLUE_COLOR = "\033[34m";

    // ANSI escape code to reset color
    const char* RESET_COLOR = "\033[0m";

    // Set up the test environment
    void SetUp() override {
        // Populating users, companies, and securities
        for (int i = 0; i < NUM_USERS; i++) {
            users.push_back("User" + std::to_string(i));
        }
        for (int i = 0; i < NUM_COMPANIES; i++) {
            companies.push_back("Comp" + std::to_string(i));
        }
        for (int i = 0; i < NUM_SECURITIES; i++) {
            secIds.push_back("SecId" + std::to_string(i));
        }
        // Using a fixed seed for reproducibility
        std::seed_seq seed{1, 2, 3, 4, 5};
        gen = std::mt19937(seed);
    }

    std::vector<Order> generateOrders(unsigned int numOrders) {
        std::vector<Order> orders;
        std::uniform_int_distribution<int> usersDist(0, users.size() - 1);
        std::uniform_int_distribution<int> companiesDist(0, companies.size() - 1);
        std::uniform_int_distribution<int> secIdsDist(0, secIds.size() - 1);
        std::uniform_int_distribution<int> sidesDist(0, sides.size() - 1);
        std::uniform_int_distribution<int> qtyDist(1, 50);

        for (int i = 0; i < numOrders; i++) {
            const auto& user = users[usersDist(gen)];
            const auto& company = companies[companiesDist(gen)];
            const auto& secId = secIds[secIdsDist(gen)];
            const auto& side = sides[sidesDist(gen)];
            unsigned int qty = qtyDist(gen) * ORDER_QTY_MULTIPLIER;

            orders.push_back(Order{"OrdId" + std::to_string(i), secId, side, qty, user, company});
        }

        return orders;
    }

    static void SetUpTestCase() {
        const char* BLUE_COLOR = "\033[34m";
        const char* RESET_COLOR = "\033[0m";

        // Calculate the Fibonacci number as the benchmark
        auto start = std::chrono::high_resolution_clock::now();
        volatile int fib = fibRecursive(30); // Calculation (Fibonacci of 30)
        auto end = std::chrono::high_resolution_clock::now();
        benchmark_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << BLUE_COLOR << "[     INFO ] 1 NCU = " << benchmark_time << "ms" << RESET_COLOR << std::endl;
    }

    static int fibRecursive(int n) {
        if (n <= 1) return n;
        return fibRecursive(n - 1) + fibRecursive(n - 2);
    }
};

double OrderCacheTest::benchmark_time = 0; // Initialize static benchmark_time

// Test U1: Add order to cache
TEST_F(OrderCacheTest, U1_UnitTest_addOrder) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"});
    cache.addOrder(Order{"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"});
    cache.addOrder(Order{"OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"});
    cache.addOrder(Order{"OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"});
    cache.addOrder(Order{"OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"});
    cache.addOrder(Order{"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"});

    ASSERT_EQ(1,1);
}

// Test U2: Get all orders from cache
TEST_F(OrderCacheTest, U2_UnitTest_getAllOrders) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"});
    cache.addOrder(Order{"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"});
    cache.addOrder(Order{"OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"});
    cache.addOrder(Order{"OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"});
    cache.addOrder(Order{"OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"});
    cache.addOrder(Order{"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"});
    std::vector<Order> allOrders = cache.getAllOrders();

    ASSERT_EQ(allOrders.size(), 8);
}

// Test U3: Cancel order
TEST_F(OrderCacheTest, U3_UnitTest_cancelOrder) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 100, "User1", "Company1"});
    cache.addOrder(Order{"OrdId2", "SecId1", "Sell", 100, "User2", "Company1"});
    std::vector<Order> allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 2);

    // Cancel order 2
    cache.cancelOrder("OrdId2");
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 1);
    ASSERT_EQ(allOrders[0].orderId(), "OrdId1");

    // Cancel order 1
    cache.cancelOrder("OrdId1");
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 0);

    // Cancel an order that does not exist
    cache.cancelOrder("OrdId3");
    ASSERT_EQ(allOrders.size(), 0);
}

// Test U4: Cancel orders for user
TEST_F(OrderCacheTest, U4_UnitTest_cancelOrdersForUser) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"});
    cache.addOrder(Order{"OrdId2", "SecId1", "Buy", 600, "User2", "CompanyB"});
    cache.addOrder(Order{"OrdId3", "SecId2", "Sell", 3000, "User1", "CompanyB"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Sell", 500, "User2", "CompanyA"});
    std::vector<Order> allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 4);

    // Cancel all orders for User1
    cache.cancelOrdersForUser("User1");
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 2); // Two orders left

    // Cancel all orders for User2
    cache.cancelOrdersForUser("User2");
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 0); // No orders left

    // Cancel an order for a user ID that does not exist
    cache.cancelOrdersForUser("User3");
    ASSERT_EQ(allOrders.size(), 0); // No orders left
}

// Test U5: Cancel orders for security with minimum quantity
TEST_F(OrderCacheTest, U5_UnitTest_cancelOrdersForSecIdWithMinimumQty) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"1", "SecId1", "Buy", 200, "User1", "Company1"});
    cache.addOrder(Order{"2", "SecId1", "Sell", 200, "User2", "Company1"});
    cache.addOrder(Order{"3", "SecId1", "Buy", 100, "User1", "Company1"});
    cache.cancelOrdersForSecIdWithMinimumQty("SecId1", 300);
    std::vector<Order> allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 3);

    // Cancel all orders with security ID 1 and minimum quantity 200
    cache.cancelOrdersForSecIdWithMinimumQty("SecId1", 200);
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 1);

    // Cancel all orders with security ID 1 and minimum quantity 100
    cache.cancelOrdersForSecIdWithMinimumQty("SecId1", 100);
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 0);
}

// Test U6: First example from README.txt
TEST_F(OrderCacheTest, U6_UnitTest_getMatchingSizeForSecurityTest_Example1) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"});
    cache.addOrder(Order{"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"});
    cache.addOrder(Order{"OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"});
    cache.addOrder(Order{"OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"});
    cache.addOrder(Order{"OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"});
    cache.addOrder(Order{"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 0);

    matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 2700);

    matchingSize = cache.getMatchingSizeForSecurity("SecId3");
    ASSERT_EQ(matchingSize, 0);
}

// Test U7: Second example from README.txt
TEST_F(OrderCacheTest, U7_UnitTest_getMatchingSizeForSecurityTest_Example2) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"OrdId1", "SecId1", "Sell", 100, "User10", "Company2"});
    cache.addOrder(Order{"OrdId2", "SecId3", "Sell", 200, "User8", "Company2"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Buy", 300, "User13", "Company2"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Sell", 400, "User12", "Company2"});
    cache.addOrder(Order{"OrdId5", "SecId3", "Sell", 500, "User7", "Company2"});
    cache.addOrder(Order{"OrdId6", "SecId3", "Buy", 600, "User3", "Company1"});
    cache.addOrder(Order{"OrdId7", "SecId1", "Sell", 700, "User10", "Company2"});
    cache.addOrder(Order{"OrdId8", "SecId1", "Sell", 800, "User2", "Company1"});
    cache.addOrder(Order{"OrdId9", "SecId2", "Buy", 900, "User6", "Company2"});
    cache.addOrder(Order{"OrdId10", "SecId2", "Sell", 1000, "User5", "Company1"});
    cache.addOrder(Order{"OrdId11", "SecId1", "Sell", 1100, "User13", "Company2"});
    cache.addOrder(Order{"OrdId12", "SecId2", "Buy", 1200, "User9", "Company2"});
    cache.addOrder(Order{"OrdId13", "SecId1", "Sell", 1300, "User1", "Company1"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 300);

    matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 1000);

    matchingSize = cache.getMatchingSizeForSecurity("SecId3");
    ASSERT_EQ(matchingSize, 600);
}

// Test U8: Third example from README.txt
TEST_F(OrderCacheTest, U8_UnitTest_getMatchingSizeForSecurityTest_Example3) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"OrdId1", "SecId3", "Sell", 100, "User1", "Company1"});
    cache.addOrder(Order{"OrdId2", "SecId3", "Sell", 200, "User3", "Company2"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Buy", 300, "User2", "Company1"});
    cache.addOrder(Order{"OrdId4", "SecId3", "Sell", 400, "User5", "Company2"});
    cache.addOrder(Order{"OrdId5", "SecId2", "Sell", 500, "User2", "Company1"});
    cache.addOrder(Order{"OrdId6", "SecId2", "Buy", 600, "User3", "Company2"});
    cache.addOrder(Order{"OrdId7", "SecId2", "Sell", 700, "User1", "Company1"});
    cache.addOrder(Order{"OrdId8", "SecId1", "Sell", 800, "User2", "Company1"});
    cache.addOrder(Order{"OrdId9", "SecId1", "Buy", 900, "User5", "Company2"});
    cache.addOrder(Order{"OrdId10", "SecId1", "Sell", 1000, "User1", "Company1"});
    cache.addOrder(Order{"OrdId11", "SecId2", "Sell", 1100, "User6", "Company2"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 900);

    matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 600);

    matchingSize = cache.getMatchingSizeForSecurity("SecId3");
    ASSERT_EQ(matchingSize, 0);
}

// Test O1: Matching Orders with Different Quantities
TEST_F(OrderCacheTest, O1_OrderMatchingTest_TestDifferentQuantities) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"1", "SecId1", "Buy", 5000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId1", "Sell", 2000, "User2", "CompanyB"});
    cache.addOrder(Order{"3", "SecId1", "Sell", 1000, "User3", "CompanyC"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 3000); // 2000 from Order 2 and 1000 from Order 3 should match with Order 1
}

// Test O2: Complex Order Combinations
TEST_F(OrderCacheTest, O2_OrderMatchingTest_TestComplexCombinations) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"1", "SecId2", "Buy", 7000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId2", "Sell", 3000, "User2", "CompanyB"});
    cache.addOrder(Order{"3", "SecId2", "Sell", 4000, "User3", "CompanyC"});
    cache.addOrder(Order{"4", "SecId2", "Buy", 500, "User4", "CompanyD"});
    cache.addOrder(Order{"5", "SecId2", "Sell", 500, "User5", "CompanyE"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 7500); // 7000 from Order 1 and 500 from Order 4 should fully match with Orders 2, 3, and 5
}

// Test O3: Orders from the Same Company Should Not Match
TEST_F(OrderCacheTest, O3_OrderMatchingTest_TestSameCompanyNoMatch) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"1", "SecId3", "Buy", 2000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId3", "Sell", 2000, "User2", "CompanyA"}); // Same company as Order 1

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId3");
    ASSERT_EQ(matchingSize, 0); // No match should occur since both orders are from the same company
}

// Test C1: Cancelling an Order that Does Not Exist
TEST_F(OrderCacheTest, C1_CancellationTest_CancelNonExistentOrder) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.cancelOrder("NonExistentOrder");
    // Assuming getAllOrders() returns all current orders, the size should be 0 for an empty cache.
    std::vector<Order> allOrders = cache.getAllOrders();
    ASSERT_TRUE(allOrders.empty());
}

// Test C2: Partial Cancellation Based on Minimum Quantity
TEST_F(OrderCacheTest, C2_CancellationTest_CancelOrdersPartialOnMinQty) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"1", "SecId1", "Buy", 200, "User1", "Company1"});
    cache.addOrder(Order{"2", "SecId1", "Sell", 500, "User2", "Company1"});
    cache.addOrder(Order{"3", "SecId1", "Buy", 300, "User3", "Company2"});

    // Cancel orders for SecId1 with a minimum quantity of 300
    cache.cancelOrdersForSecIdWithMinimumQty("SecId1", 300);

    std::vector<Order> allOrders = cache.getAllOrders();
    // Only the order with quantity 200 should remain
    ASSERT_EQ(allOrders.size(), 1);
    ASSERT_EQ(allOrders[0].orderId(), "1");
}

// Test C3: Cancelling Multiple Orders for the Same User or Security ID
TEST_F(OrderCacheTest, C3_CancellationTest_CancelMultipleOrdersForUser) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"1", "SecId1", "Buy", 200, "User1", "Company1"});
    cache.addOrder(Order{"2", "SecId2", "Sell", 300, "User1", "Company1"});
    cache.addOrder(Order{"3", "SecId3", "Buy", 400, "User2", "Company2"});

    // Cancel all orders for User1
    cache.cancelOrdersForUser("User1");

    std::vector<Order> allOrders = cache.getAllOrders();
    // Only the order for User2 should remain
    ASSERT_EQ(allOrders.size(), 1);
    ASSERT_EQ(allOrders[0].orderId(), "3");
}

// Test M1: Multiple Small Orders Matching a Larger Order
TEST_F(OrderCacheTest, M1_MatchingSizeTest_MultipleSmallOrdersMatchingLargeOrder) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"1", "SecId1", "Buy", 10000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId1", "Sell", 2000, "User2", "CompanyB"});
    cache.addOrder(Order{"3", "SecId1", "Sell", 1500, "User3", "CompanyC"});
    cache.addOrder(Order{"4", "SecId1", "Sell", 2500, "User4", "CompanyD"});
    cache.addOrder(Order{"5", "SecId1", "Sell", 4000, "User5", "CompanyE"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 10000); // Total of 10000 from orders 2, 3, 4, and 5 should match with order 1
}

// Test M2: Multiple Matching Combinations
TEST_F(OrderCacheTest, M2_MatchingSizeTest_MultipleMatchingCombinations) {
    CHECK_GLOBAL_FAILURE_FLAG();

    cache.addOrder(Order{"1", "SecId2", "Buy", 6000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId2", "Sell", 2000, "User2", "CompanyB"});
    cache.addOrder(Order{"3", "SecId2", "Sell", 3000, "User3", "CompanyC"});
    cache.addOrder(Order{"4", "SecId2", "Buy", 1000, "User4", "CompanyD"});
    cache.addOrder(Order{"5", "SecId2", "Sell", 1500, "User5", "CompanyE"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 6500); // Total of 6500 (2000 from Order 2, 3000 from Order 3, 1500 from Order 5) should match with Orders 1 and 4
}

// Test P1: Add and match 1,000 orders
TEST_F(OrderCacheTest, P1_PerfTest_1000_Orders) {
    CHECK_GLOBAL_FAILURE_FLAG();

    unsigned int NUM_ORDERS = 1000;
    std::vector<Order> orders = generateOrders(NUM_ORDERS);
    auto start = std::chrono::high_resolution_clock::now();

    // Add orders to the cache
    for (int i = 0; i < orders.size(); i++) {
        cache.addOrder(orders[i]);
    }

    // Get matching size for every security and measure the time it takes
    for (const auto& secId : secIds) {
        cache.getMatchingSizeForSecurity(secId);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double ncu = duration / benchmark_time;  // Calculate the NCU based on benchmark

    // Display the result
    std::cout << BLUE_COLOR << "[     INFO ] Matched " << NUM_ORDERS << " orders in " << ncu << " NCUs (" << duration << "ms)" << RESET_COLOR << std::endl;
    ASSERT_LE(ncu, 1500);
}

// Test P2: Add and match 5,000 orders
TEST_F(OrderCacheTest, P2_PerfTest_5000_Orders) {
    CHECK_GLOBAL_FAILURE_FLAG();

    unsigned int NUM_ORDERS = 5000;
    std::vector<Order> orders = generateOrders(NUM_ORDERS);
    auto start = std::chrono::high_resolution_clock::now();

    // Add orders to the cache
    for (int i = 0; i < orders.size(); i++) {
        cache.addOrder(orders[i]);
    }

    // Get matching size for every security and measure the time it takes
    for (const auto& secId : secIds) {
        cache.getMatchingSizeForSecurity(secId);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double ncu = duration / benchmark_time;  // Calculate the NCU based on benchmark

    // Display the result
    std::cout << BLUE_COLOR << "[     INFO ] Matched " << NUM_ORDERS << " orders in " << ncu << " NCUs (" << duration << "ms)" << RESET_COLOR << std::endl;
    ASSERT_LE(ncu, 1500);
}

// Test P3: Add and match 10,000 orders
TEST_F(OrderCacheTest, P3_PerfTest_10000_Orders) {
    CHECK_GLOBAL_FAILURE_FLAG();

    unsigned int NUM_ORDERS = 10000;
    std::vector<Order> orders = generateOrders(NUM_ORDERS);
    auto start = std::chrono::high_resolution_clock::now();

    // Add orders to the cache
    for (int i = 0; i < orders.size(); i++) {
        cache.addOrder(orders[i]);
    }

    // Get matching size for every security and measure the time it takes
    for (const auto& secId : secIds) {
        cache.getMatchingSizeForSecurity(secId);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double ncu = duration / benchmark_time;  // Calculate the NCU based on benchmark

    // Display the result
    std::cout << BLUE_COLOR << "[     INFO ] Matched " << NUM_ORDERS << " orders in " << ncu << " NCUs (" << duration << "ms)" << RESET_COLOR << std::endl;
    ASSERT_LE(ncu, 1500);
}

// Test P4: Add and match 50,000 orders
TEST_F(OrderCacheTest, P4_PerfTest_50000_Orders) {
    CHECK_GLOBAL_FAILURE_FLAG();

    unsigned int NUM_ORDERS = 50000;
    std::vector<Order> orders = generateOrders(NUM_ORDERS);
    auto start = std::chrono::high_resolution_clock::now();

    // Add orders to the cache
    for (int i = 0; i < orders.size(); i++) {
        cache.addOrder(orders[i]);
    }

    // Get matching size for every security and measure the time it takes
    for (const auto& secId : secIds) {
        cache.getMatchingSizeForSecurity(secId);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double ncu = duration / benchmark_time;  // Calculate the NCU based on benchmark

    // Display the result
    std::cout << BLUE_COLOR << "[     INFO ] Matched " << NUM_ORDERS << " orders in " << ncu << " NCUs (" << duration << "ms)" << RESET_COLOR << std::endl;
    ASSERT_LE(ncu, 1500);
}

// Test P5: Add and match 10,000 orders
TEST_F(OrderCacheTest, P5_PerfTest_100000_Orders) {
    CHECK_GLOBAL_FAILURE_FLAG();

    unsigned int NUM_ORDERS = 100000;
    std::vector<Order> orders = generateOrders(NUM_ORDERS);
    auto start = std::chrono::high_resolution_clock::now();

    // Add orders to the cache
    for (int i = 0; i < orders.size(); i++) {
        cache.addOrder(orders[i]);
    }

    // Get matching size for every security and measure the time it takes
    for (const auto& secId : secIds) {
        cache.getMatchingSizeForSecurity(secId);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double ncu = duration / benchmark_time;  // Calculate the NCU based on benchmark

    // Display the result
    std::cout << BLUE_COLOR << "[     INFO ] Matched " << NUM_ORDERS << " orders in " << ncu << " NCUs (" << duration << "ms)" << RESET_COLOR << std::endl;
    ASSERT_LE(ncu, 1500);
}

// Test P6: Add and match 500,000 orders
TEST_F(OrderCacheTest, P6_PerfTest_500000_Orders) {
    CHECK_GLOBAL_FAILURE_FLAG();

    unsigned int NUM_ORDERS = 500000;
    std::vector<Order> orders = generateOrders(NUM_ORDERS);
    auto start = std::chrono::high_resolution_clock::now();

    // Add orders to the cache
    for (int i = 0; i < orders.size(); i++) {
        cache.addOrder(orders[i]);
    }

    // Get matching size for every security and measure the time it takes
    for (const auto& secId : secIds) {
        cache.getMatchingSizeForSecurity(secId);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double ncu = duration / benchmark_time;  // Calculate the NCU based on benchmark

    // Display the result
    std::cout << BLUE_COLOR << "[     INFO ] Matched " << NUM_ORDERS << " orders in " << ncu << " NCUs (" << duration << "ms)" << RESET_COLOR << std::endl;
    ASSERT_LE(ncu, 1500);
}

// Test P7: Add and match 1,000,000 orders
TEST_F(OrderCacheTest, P7_PerfTest_1000000_Orders) {
    CHECK_GLOBAL_FAILURE_FLAG();

    unsigned int NUM_ORDERS = 1000000;
    std::vector<Order> orders = generateOrders(NUM_ORDERS);
    auto start = std::chrono::high_resolution_clock::now();

    // Add orders to the cache
    for (int i = 0; i < orders.size(); i++) {
        cache.addOrder(orders[i]);
    }

    // Get matching size for every security and measure the time it takes
    for (const auto& secId : secIds) {
        cache.getMatchingSizeForSecurity(secId);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    double ncu = duration / benchmark_time;  // Calculate the NCU based on benchmark

    // Display the result
    std::cout << BLUE_COLOR << "[     INFO ] Matched " << NUM_ORDERS << " orders in " << ncu << " NCUs (" << duration << "ms)" << RESET_COLOR << std::endl;
    ASSERT_LE(ncu, 1500);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Custom failure listener
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new FailureListener);

    return RUN_ALL_TESTS();
}
