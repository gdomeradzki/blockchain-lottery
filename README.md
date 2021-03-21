# Blockchain lottery
## POC of blockchain lottery

This is a POC of blockchain lottery without a need of third party. 

## Details

- Nodes maintain their own blockchains, registering all events regarding lottery and money transfer
- Network doesn't require 3rd party "lottery server"
- In this version nodes are controlled by main program thread but they are adjusted to work in distributed network environment. Also lottery event can be determined by protocol eg. once a day
- Algorithm which selects the winner is simple and can be improved but for this POC is fair enough
```
int sum = 0;
std::for_each(
    lotteryInfo.reveals.begin(), lotteryInfo.reveals.end(), [&sum](const auto& reveal) { sum += reveal.bet; });
int winningIndex = sum % COMBINATIONS;
```
- Before above result calculations, nodes don't know each other bets and provides only hash to the network:
```
int playerBet = std::rand() % COMBINATIONS;
int salt = std::rand() % 1000000;
int valueForHash = playerBet + salt;
int betHash = std::hash<int>{}(valueForHash);
```
- After some time, network doesn't accept any more bets and waits for "reveals". Nodes are obligated to provide thier bets and salt to confirm hashes they provided earlier
- If nobody wins, the pool goes to the next lottery
- Winning probability in this implementations is like 1:100
- Player may not join to the lottery and skip a round
- Basic security checks are performed (player cannot join without money or reveal bet without join)
## For the future
- Implement distributed network to mutual maintenance of blockchain
- Use asymmetric cryptography for nodes authorizations and their actions
- Provide some file format for exchanging information via network
