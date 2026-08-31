namespace HeroHour.Prototype

module PrototypeContent =
    let private unit id name faction role cost buildTime hitPoints damage moveSpeed = {
        Id = id
        DisplayName = name
        Faction = faction
        Role = role
        Cost = cost
        BuildTimeSeconds = buildTime
        HitPoints = hitPoints
        Damage = damage
        MoveSpeed = moveSpeed
    }

    let map = {
        Id = "prototype_crossroads"
        DisplayName = "Crossroads"
        RecommendedPlayers = 2
        StartingCredits = 2500
    }

    let units = [
        unit "hero_ranger" "HERO Ranger" Hero Infantry 300 10 100 12 5.0
        unit "hero_guardian" "HERO Guardian" Hero Vehicle 850 24 520 45 2.8
        unit "hero_falcon" "HERO Falcon" Hero Air 1100 30 260 34 7.5
        unit "usa_rifleman" "USA Rifleman" USA Infantry 280 9 90 11 5.2
        unit "usa_tank" "USA Battle Tank" USA Vehicle 900 25 560 48 2.6
        unit "usa_gunship" "USA Gunship" USA Air 1150 32 280 36 7.0
    ]

    let scope = {
        Map = map
        Factions = [ Hero; USA ]
        Units = units
    }

    let unitsFor faction =
        units |> List.filter (fun definition -> definition.Faction = faction)

    let validateScope (prototype: PrototypeScope) =
        let expectedRoles = Set.ofList [ Infantry; Vehicle; Air ]

        [
            if prototype.Map.RecommendedPlayers <> 2 then
                "The prototype map must be a 1v1 map."

            if Set.ofList prototype.Factions <> Set.ofList [ Hero; USA ] then
                "The prototype must contain exactly HERO and USA."

            let duplicateIds =
                prototype.Units
                |> List.countBy (fun definition -> definition.Id)
                |> List.filter (fun (_, count) -> count > 1)

            if not duplicateIds.IsEmpty then
                "Every unit must have a unique id."

            for faction in prototype.Factions do
                let factionUnits = prototype.Units |> List.filter (fun unit -> unit.Faction = faction)
                let factionRoles = factionUnits |> List.map (fun unit -> unit.Role) |> Set.ofList

                if factionUnits.Length <> 3 || factionRoles <> expectedRoles then
                    $"{faction} must have exactly one infantry, vehicle, and air unit."

            for definition in prototype.Units do
                if definition.Cost <= 0 || definition.BuildTimeSeconds <= 0 then
                    $"{definition.Id} must have a positive cost and build time."

                if definition.HitPoints <= 0 || definition.Damage <= 0 || definition.MoveSpeed <= 0.0 then
                    $"{definition.Id} must have positive combat stats."
        ]
