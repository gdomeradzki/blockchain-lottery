#include <ctime>
#include <iostream>
#include "Node.hpp"

using namespace std;

void printNetworkBalance(PlayerId playerIdData, std::map<PlayerId, Cash> balance)
{
    std::cout << "Player #" << playerIdData << " claims: " << std::endl;
    for (const auto& [id, cash] : balance)
    {
        std::cout << id << " -> " << cash << "$" << std::endl;
    }
}

void performSimulation(int nodesNumber, int numberOfSimulations)
{
    Node nodes[nodesNumber];
    for (int i = 0; i < nodesNumber; i++)
    {
        EventMoneyTransfer startMoney{NOBODY_PLAYER_ID, nodes[i].getPlayerId(), 100};
        for (int j = 0; j < nodesNumber; j++)
        {
            nodes[j].newTransferInNetwork(startMoney);
        }
    }

    for (int i = 0; i < numberOfSimulations; i++)
    {
        std::cout << "----------------------------------------\nLottery number #" << i << std::endl;
        for (int j = 0; j < nodesNumber; j++)
        {
            const auto joinsToLottery = nodes[j].joinToLottery();
            if (joinsToLottery)
            {
                for (int k = 0; k < nodesNumber; k++)
                {
                    nodes[k].newPlayerJoinLottery(joinsToLottery.value());
                }
            }
        }

        for (int j = 0; j < nodesNumber; j++)
        {
            const auto revealBet = nodes[j].networkCollectedAllBets();
            if (revealBet)
            {
                for (int k = 0; k < nodesNumber; k++)
                {
                    nodes[k].revealPlayerBet(revealBet.value());
                }
            }
        }
    }
    for (int i = 0; i < nodesNumber; i++)
    {
        printNetworkBalance(nodes[i].getPlayerId(), nodes[i].showNetworkBalance());
    }
}

int main()
{
    std::srand(std::time(nullptr));
    performSimulation(15, 20);

    return 0;
}
