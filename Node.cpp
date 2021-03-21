#include "Node.hpp"

#include <algorithm>
#include <iostream>
#include <set>
template <class... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...)->overload<Ts...>;

Node::Node()
{
    static PlayerId nextPlayerId{1};
    playerId = nextPlayerId++;
    blockChain.push_back(EventLotteryEnd{-1, 0});
}

void Node::newPlayerJoinLottery(const EventPlayerJoin& playerJoin)
{
    auto networkBalance = showNetworkBalance();
    if (networkBalance.at(playerJoin.playerId) < BET_MONEY_VALUE)
    {
        std::cout << "Player #" << playerId << " says: "
                  << "Error! Player #" << playerJoin.playerId << " cannot join lottery (not enough money)!"
                  << std::endl;
        return;
    }

    auto lotteryInfo = currentLotteryInfo();
    if (std::find_if(lotteryInfo.bets.begin(), lotteryInfo.bets.end(), [&playerJoin](const auto& bet) {
            return bet.playerId == playerJoin.playerId;
        }) != lotteryInfo.bets.end())
    {
        std::cout << "Player #" << playerId << " says: "
                  << "Error! Player #" << playerJoin.playerId << " cannot make double bet!" << std::endl;
        return;
    }
    std::cout << "Player #" << playerId << " says: "
              << "New player #" << playerJoin.playerId << " joined the lottery" << std::endl;
    blockChain.push_back(playerJoin);
    EventMoneyTransfer event{playerJoin.playerId, NOBODY_PLAYER_ID, BET_MONEY_VALUE};
    blockChain.push_back(event);
}

void Node::selectWinner(const LotteryInfo& lotteryInfo)
{
    int sum = 0;
    std::for_each(
        lotteryInfo.reveals.begin(), lotteryInfo.reveals.end(), [&sum](const auto& reveal) { sum += reveal.bet; });
    int winningIndex = sum % COMBINATIONS;
    auto lotteryResult = lotteryInfo.prevLottery;
    lotteryResult.leftPool += lotteryInfo.bets.size() * BET_MONEY_VALUE;
    if (winningIndex >= lotteryInfo.bets.size())
    {
        std::cout << "Player #" << playerId << " says: "
                  << "Nobody won! New pool is: " << lotteryResult.leftPool << std::endl;
        lotteryResult.winner = NOBODY_PLAYER_ID;
    }
    else
    {
        PlayerId winnerId = lotteryInfo.bets.at(winningIndex).playerId;
        std::cout << "Player #" << playerId << " says: "
                  << "There is a win! Player #" << winnerId << " won: " << lotteryResult.leftPool << std::endl;
        lotteryResult.winner = winningIndex;
        EventMoneyTransfer event{NOBODY_PLAYER_ID, winnerId, lotteryResult.leftPool};
        blockChain.push_back(event);
        lotteryResult.leftPool = 0;
    }
    blockChain.push_back(lotteryResult);
}

void Node::revealPlayerBet(const EventPlayerRevealBet& playerBet)
{
    auto bet = findBetInCurrentLottery(playerBet.playerId);
    if (!bet)
    {
        std::cout << "Player #" << playerId << " says: "
                  << "Warning! Player #" << playerBet.playerId
                  << " revealed his bet but didn't enter the lottery or his ticket has been refused!" << std::endl;
        return;
    }
    if (!checkBetHashes(playerBet.bet, playerBet.salt, *bet))
    {
        std::cout << playerBet.bet << " " << playerBet.salt << " " << bet->betHash << " " << bet->playerId << std::endl;
        std::cout << "Player #" << playerId << " says: "
                  << "Error! Player #" << playerBet.playerId << " provided wrong hash!" << std::endl;
        return;
    }

    blockChain.push_back(playerBet);
    auto lotteryInfo = currentLotteryInfo();
    if (lotteryInfo.bets.size() == lotteryInfo.reveals.size())
    {
        selectWinner(lotteryInfo);
    }
}

void Node::newTransferInNetwork(const EventMoneyTransfer& moneyTransfer)
{
    blockChain.push_back(moneyTransfer);
}

PlayerId Node::getPlayerId() const
{
    return playerId;
}

std::optional<EventPlayerJoin> Node::joinToLottery()
{
    if (std::rand() % 5 != 0)
    {
        int playerBet = std::rand() % COMBINATIONS;
        int salt = std::rand() % 1000000;
        int valueForHash = playerBet + salt;
        int betHash = std::hash<int>{}(valueForHash);
        lastBet = EventPlayerRevealBet{playerId, playerBet, salt};
        EventPlayerJoin eventPlayerJoin{playerId, betHash};

        std::cout << "Player #" << playerId << " says: "
                  << "Player #" << playerId << " decided to join lottery with hash: " << betHash << std::endl;
        return eventPlayerJoin;
    }
    std::cout << "Player #" << playerId << " says: "
              << "Player #" << playerId << " says: "
              << "Player #" << playerId << " decided to skip the lottery" << std::endl;
    return std::nullopt;
}

std::optional<EventPlayerRevealBet> Node::networkCollectedAllBets()
{
    blockChain.push_back(EventBetsEnd{});
    if (lastBet)
    {
        return lastBet;
    }
    return std::nullopt;
}

std::map<PlayerId, Cash> Node::showNetworkBalance() const
{
    std::map<PlayerId, Cash> networkBalance;
    for (int i = blockChain.size() - 1; i >= 0; i--)
    {
        std::visit(
            overload{[&networkBalance](const EventMoneyTransfer& eventFromPast) {
                         if (eventFromPast.to != NOBODY_PLAYER_ID)
                         {
                             networkBalance[eventFromPast.to] += eventFromPast.amount;
                         }
                         if (eventFromPast.from != NOBODY_PLAYER_ID)
                         {
                             networkBalance[eventFromPast.from] -= eventFromPast.amount;
                         }
                     },
                     [](const auto&) {}},
            blockChain.at(i));
    }
    return networkBalance;
}

std::optional<EventPlayerJoin> Node::findBetInCurrentLottery(int playerId) const
{
    std::optional<EventPlayerJoin> event;
    for (int i = blockChain.size() - 1; i >= 0; i--)
    {
        std::visit(
            overload{[&i](const EventLotteryEnd&) { i = -1; },
                     [&event, &i, playerId](const EventPlayerJoin& eventFromPast) {
                         if (eventFromPast.playerId == playerId)
                         {
                             event = eventFromPast;
                             i = -1;
                         }
                     },
                     [](const auto&) {}},
            blockChain.at(i));
    }
    return event;
}

bool Node::checkBetHashes(int bet, int salt, const EventPlayerJoin& playerJoin) const
{
    int hashValue = bet + salt;
    return std::hash<int>{}(hashValue) == playerJoin.betHash;
}

Node::LotteryInfo Node::currentLotteryInfo() const
{
    LotteryInfo lotteryInfo;
    for (int i = blockChain.size() - 1; i >= 0; i--)
    {
        std::visit(
            overload{
                [&lotteryInfo](const EventPlayerJoin& eventFromPast) { lotteryInfo.bets.push_back(eventFromPast); },
                [&lotteryInfo](const EventPlayerRevealBet& eventFromPast) {
                    lotteryInfo.reveals.push_back(eventFromPast);
                },
                [&lotteryInfo, &i](const EventLotteryEnd& event) {
                    lotteryInfo.prevLottery = event;
                    i = -1;
                },
                [](const auto&) {}},
            blockChain.at(i));
    }
    return lotteryInfo;
}
