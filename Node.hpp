#pragma once
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

using PlayerId = int;
using Cash = int;

constexpr auto COMBINATIONS = 100;
constexpr auto BET_MONEY_VALUE = 5;
struct EventPlayerJoin
{
    PlayerId playerId;
    int betHash;
};

struct EventPlayerRevealBet
{
    PlayerId playerId;
    int bet;
    int salt;
};

struct EventBetsEnd
{
};
constexpr auto NOBODY_PLAYER_ID = -1;
struct EventLotteryEnd
{
    PlayerId winner;
    Cash leftPool;
};

struct EventMoneyTransfer
{
    PlayerId from;
    PlayerId to;
    int amount;
};
using Event = std::variant<EventMoneyTransfer, EventPlayerJoin, EventPlayerRevealBet, EventBetsEnd, EventLotteryEnd>;
using BlockChain = std::vector<Event>;

class Node
{
public:
    Node();
    void newPlayerJoinLottery(const EventPlayerJoin& playerJoin);
    void revealPlayerBet(const EventPlayerRevealBet& playerBet);
    void newTransferInNetwork(const EventMoneyTransfer& moneyTransfer);
    PlayerId getPlayerId() const;
    std::optional<EventPlayerJoin> joinToLottery();
    std::optional<EventPlayerRevealBet> networkCollectedAllBets();

    std::map<PlayerId, Cash> showNetworkBalance() const;

private:
    std::optional<EventPlayerJoin> findBetInCurrentLottery(int playerId) const;
    bool checkBetHashes(int bet, int salt, const EventPlayerJoin& playerJoin) const;

    struct LotteryInfo
    {
        std::vector<EventPlayerJoin> bets;
        std::vector<EventPlayerRevealBet> reveals;
        EventLotteryEnd prevLottery;
    };

    LotteryInfo currentLotteryInfo() const;
    void selectWinner(const LotteryInfo& lotteryInfo);

    std::optional<EventPlayerRevealBet> lastBet;

    PlayerId playerId;
    BlockChain blockChain;
};
