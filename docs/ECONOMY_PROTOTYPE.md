# Salvage economy and base production

The prototype now has a native economy module in `PrototypeEconomy.cpp`. Integration, assets and live validation are handled by the main development agent.

## Player loop

Start with 800 credits, an HQ, a completed depot, three combat pickups and one collector. Collectors automatically drive to the nearest salvage stock, load ten credits every half-second up to a 100-credit capacity, return to a completed depot and deliver the cargo. Credits arrive only through delivery; there is no passive income. Resource stocks decrease during harvesting and restart restores them.

The player builds a factory to produce combat vehicles. Building placement must be within 800 units of the friendly HQ, within the road corridor, and clear of other vehicles, structures and salvage stocks. Buildings are vulnerable while under construction. A factory or depot cannot function until construction finishes.

| Order | Cost | Time | Producer |
|---|---:|---:|---|
| Depot | 200 | 8 seconds | Place near HQ |
| Factory | 350 | 12 seconds | Place near HQ |
| Combat pickup | 100 | 5 seconds | Completed factory |
| Collector | 150 | 6 seconds | Completed HQ |

Each production building accepts five queued orders. Payment occurs when the order is accepted. Each producer processes one order at a time; blocked exits delay deployment rather than losing the queued unit. The army cap of 40 includes paid queued vehicles. Destroyed producers lose their queued orders.

## Enemy loop

The opponent starts with the same 800 credits and a completed HQ/depot/factory, three combat pickups and a collector. It harvests the same finite salvage stocks, pays the same production costs, and attempts to replace missing infrastructure. During the first 60 seconds it builds a defensive force capped at six combat vehicles, including paid queued vehicles, so the player has time to construct a factory. It begins HQ attack orders after this setup period. It receives no free waves or passive income.

## Verification

The `-PrototypeEconomyTest` command should be dispatched by the game mode at startup. It exercises actual economy methods for harvesting, delivery, placement rejection, timed construction, required production buildings, insufficient credit rejection, paid queue capacity, timed spawning and restart. The existing combat test retains live timed damage and verifies movement orders, strategic HQ targeting and match outcomes alongside the new economy integration.

These tests do not replace live inspection of collector routes, building preview, unit spacing, HUD clicks or production exits. Those require the rendered prototype.

Verification on 11 October 2026: the editor target compiled successfully. `Saved/Logs/EconomyMovementTest.log` reports `ECONOMY_TEST_RESULT=PASS`, including a loaded collector traveling using real unit movement ticks to deliver at the depot, without teleporting to the delivery point. `Saved/Logs/EconomyIntegratedCombatTest.log` reports `COMBAT_TEST_RESULT=PASS` for the integrated combat and restart checks. Both runs used the imported prototype map with the command-line editor and no renderer.

After balancing, `Saved/Logs/EconomyBalanceTest.log` also reports `ECONOMY_TEST_RESULT=PASS`, including explicit checks for the 60-second grace period, capped paid defenses, and attack orders when the grace period expires. The integrated combat test was rerun successfully with the new 13-actor starting composition. Salvage stocks are at x = -650, 650 and 0, y = 400, making the side-depot transport distance 750 units before delivery range is considered.
