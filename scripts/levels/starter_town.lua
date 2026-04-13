function init(self)
    self.npcIDs = {}
    self.npcDirections = {}

    self.directionTimer = 0
    self.newNPCTimer = 0

    self.spawnNPC = function(self, x, y)
        print("spawnNPC")

        local npcID = Entity.Create(x, y)

        print("Created entity with ID" .. npcID)

        Entity.SetActive(npcID, true)

        print(npcID)

        Entity.AddAnimationComponent(npcID, "animation/npc_man_1.xml", AnimationName.IDLE_DOWN)
        Entity.AddGridMovementComponent(npcID)
        Entity.AddScriptComponent(npcID, "../scripts/entities/test_script_1.lua")

        table.insert(self.npcIDs, npcID)
        table.insert(self.npcDirections, Direction.North)
    end

    self.onActivate = function(self)
        print("onActivate")

        self:spawnNPC(736.0, 384.0)
    end

    self.onDeactivate = function(self)
        print("onDeactivate")

        for i, v in ipairs(self.npcIDs) do
            Entity.Destroy(v)
        end
    end

    self.update = function(self, dt)
        self.directionTimer = self.directionTimer + dt
        self.newNPCTimer = self.newNPCTimer + dt

        if self.directionTimer > 1 then
            for i, v in ipairs(self.npcIDs) do
                self.npcDirections[i] = math.random(0, 3)
            end

            self.directionTimer = 0
        end

        if self.newNPCTimer > 5 then
            for i = 1, 3 do
                self:spawnNPC(736.0, 384.0)
            end
            self.newNPCTimer = 0
        end

        for i, v in ipairs(self.npcIDs) do
            if Entity.CanMove(v, self.npcDirections[i]) then
                Entity.Move(v, self.npcDirections[i])
            else
                if self.npcDirections[i] == Direction.North then
                    self.npcDirections[i] = Direction.South

                elseif self.npcDirections[i] == Direction.South then
                    self.npcDirections[i] = Direction.North

                elseif self.npcDirections[i] == Direction.East then
                    self.npcDirections[i] = Direction.West

                else
                    self.npcDirections[i] = Direction.East

                end
            end
        end
    end
end
