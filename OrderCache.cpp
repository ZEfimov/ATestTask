// Todo: your implementation of the OrderCache...
#include "OrderCache.h"
#include <algorithm>
#include <iostream>
#include <deque>

void OrderCache::addOrder(Order order) 
{
  const auto& id = order.orderId();
  const auto& user = order.user();
  const auto& secId = order.securityId();
  const auto isBuy = order.isBuy();

  m_ordersById[id] = std::make_shared<Order>(std::move(order));
  m_ordersByUser[user][id] = m_ordersById[id];
  if(isBuy)
  {
    m_ordersBySecurity_BuySell[secId].first[id] = m_ordersById[id];
  }
  else
  {
    m_ordersBySecurity_BuySell[secId].second[id] = m_ordersById[id];
  }
}

void OrderCache::cancelOrder(const std::string& orderId) 
{
  DeleteOrder(orderId);
}

void OrderCache::cancelOrdersForUser(const std::string& user) 
{
  std::deque<std::string> toRemove;
  for(auto& order:m_ordersByUser[user])
  {
    toRemove.push_back(order.first);
  }
  for(auto& order:toRemove)
  {
    DeleteOrder(order);
  }

}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) 
{
  auto it = m_ordersBySecurity_BuySell.find(securityId);
  if (it == m_ordersBySecurity_BuySell.end())
  {
    return;
  }
  std::deque<std::string> toRemove;
  for(const auto&[secId, orders]:it->second.first)
  {
    if (orders->qty() >= minQty)
    {
      toRemove.push_back(orders->orderId());
    }
  }
  for(const auto&[secId, orders]:it->second.second)
  {
    if (orders->qty() >= minQty)
    {
      toRemove.push_back(orders->orderId());
    }
  }
  for(const auto& id: toRemove)
  {
    DeleteOrder(id);
  }
}
unsigned int OrderCache::getMatchingSizeForSecurity(const std::string& securityId) 
{
    unsigned int totalMatchQty = 0;
    auto it = m_ordersBySecurity_BuySell.find(securityId);
    if (it == m_ordersBySecurity_BuySell.end()) 
    {
        return totalMatchQty;
    }

    const auto& [buyOrders, sellOrders] = it->second;

    std::unordered_map<std::string, unsigned int> aggregatedBuy;
    std::unordered_map<std::string, unsigned int> aggregatedSell;
    aggregatedBuy.reserve(buyOrders.size());
    aggregatedSell.reserve(sellOrders.size());
    
    // Aggregate quantities by company
    for (const auto& [id, order] : buyOrders) 
    {
        aggregatedBuy[order->company()] += order->qty();
    }
    for (const auto& [id, order] : sellOrders) 
    {
        aggregatedSell[order->company()] += order->qty();
    }

    // Match buy and sell orders between different companies
    for (auto& [buyCompany, buyQty] : aggregatedBuy) 
    {
        for (auto& [sellCompany, sellQty] : aggregatedSell) 
        {
            if (buyCompany != sellCompany) 
            {
                auto matchQty = std::min(buyQty, sellQty);
                totalMatchQty += matchQty;
                buyQty -= matchQty;
                sellQty -= matchQty;
            }
        }
    }
    return totalMatchQty;
}

std::vector<Order> OrderCache::getAllOrders() const 
{
  auto result = std::vector<Order>();
  result.reserve(m_ordersById.size());
  for (const auto& [orderId, order] : m_ordersById) 
  {
      result.emplace_back(*order);
  }
  return result;
}

void OrderCache::DeleteOrder(const std::string& id)
{
  auto it = m_ordersById.find(id);
  if (it == m_ordersById.end())
  {
    return;
  }
  const Order& order = *(it->second.get());
  const std::string& secId =  order.securityId();
  const std::string& user =  order.user();

  m_ordersByUser[user].erase(id);
  if(order.isBuy())
  {
    m_ordersBySecurity_BuySell[secId].first.erase(id);
  }
  else
  {
    m_ordersBySecurity_BuySell[secId].second.erase(id);
  }
  m_ordersById.erase(id);
}