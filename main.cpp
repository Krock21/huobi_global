#include <bits/stdc++.h>
#include "json.hpp"

using namespace std;

// for convenience
using json = nlohmann::json;

class Price {
  int leftSide;
  int rightSide;

  Price() {

  }

public:
  Price(int leftSide, int rightSide) : leftSide(leftSide), rightSide(rightSide) {

  }

  ~Price() = default;

  Price(const Price &other) = default;

  Price(Price &&other) noexcept = default;

  Price &operator=(const Price &other) = default;

  Price &operator=(Price &&other) noexcept = default;

  explicit Price(const string &s) {
    int dotPosition = s.find('.');
    if (dotPosition == string::npos) {
      leftSide = stoi(s);
      rightSide = 0;
    } else {
      leftSide = stoi(s.substr(0, dotPosition));
      rightSide = stoi(s.substr(dotPosition + 1));
    }
  }

  string toString() {
    return to_string(leftSide) + "." + to_string(rightSide);
  }

  bool operator<(const Price &price) const {
    if (leftSide < price.leftSide) {
      return true;
    } else if (leftSide > price.leftSide) {
      return false;
    } else {
      return rightSide < price.rightSide;
    }
  }
};

class SimpleHolder {
  map<Price, string> bids;
  map<Price, string> asks;

public:
  void addBid(const json &bid) {
    Price bid_price = Price(bid[0].dump());
    string bid_value = bid[1].dump();
    if (stof(bid_value) == 0) {
      bids.erase(bid_price);
    } else {
      bids[bid_price] = bid_value;
    }
  }

  void addAsk(const json &ask) {
    Price ask_price = Price(ask[0].dump());
    string ask_value = ask[1].dump();
    if (stof(ask_value) == 0) {
      asks.erase(ask_price);
    } else {
      asks[ask_price] = ask_value;
    }
  }

  pair<Price, string> getBestBid() {
    if (bids.empty()) {
      return {Price("0"), "0"};
    }
    auto it = bids.begin();
    return {it->first, it->second};
  }

  pair<Price, string> getBestAsk() {
    if (asks.empty()) {
      return {Price("0"), "0"};
    }
    auto it = asks.rbegin();
    return {it->first, it->second};
  }
};

template<typename Holder>
class OrderBook {
  Holder holder;

public:
  void processSnapshot(const json &snapshot) {
    for (const json &bid : snapshot["bids"]) {
      holder.addBid(bid);
    }
    for (const json &ask : snapshot["asks"]) {
      holder.addAsk(ask);
    }
  }

  void processUpdate(const json &update) {
    for (const json &bid : update["bids"]) {
      holder.addBid(bid);
    }
    for (const json &ask : update["asks"]) {
      holder.addAsk(ask);
    }
  }

  void processQuery(const json &query) {
    if (query["type"] == "snapshot") {
      processSnapshot(query);
    } else if (query["type"] == "update") {
      processUpdate(query);
    } else {
      throw runtime_error("Unknown query type");
    }
  }

  pair<pair<Price, string>, pair<Price, string>> getBestValues() {
    return make_pair(holder.getBestBid(), holder.getBestAsk());
  }
};

int main(int argc, char **argv) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " log_file";
    return 1;
  }
  ifstream fin(argv[1]);
  OrderBook<SimpleHolder> book;
  string s;
  while (getline(fin, s)) {
    s = s.substr(string("[2020-08-23 19:19:29.137] [exchange_tools] [info] Get Object: ").size());
    json query = json::parse(s);
    book.processQuery(query);
    auto bestValues = book.getBestValues();
    std::cout << query["event_time"] << ", "
              << bestValues.first.first.toString() << ", " << bestValues.first.second << ", "
              << bestValues.second.first.toString() << ", " << bestValues.second.second << "\n";
  }
}