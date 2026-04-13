function init(self)
    self.npcID1 = 0
    self.currentDirectionNPC1 = Direction.North

    self.directionTimer = 0

    self.onActivate = function(self)
        print("onActivate")

        self.npcID1 = Entity.Create(736.0, 384.0)

        Entity.SetActive(self.npcID1, true)

        print(self.npcID1)

        Entity.AddAnimationComponent(self.npcID1, "animation/npc_man_1.xml", AnimationName.IDLE_DOWN)
        Entity.AddGridMovementComponent(self.npcID1)
        Entity.AddScriptComponent(self.npcID1, "../scripts/entities/test_script_1.lua")
    end

    self.onDeactivate = function(self)
        print("onDeactivate")
        Entity.Destroy(self.npcID1)
    end

    self.update = function(self, dt)
        self.directionTimer = self.directionTimer + dt

        if self.directionTimer > 2 then
            self.currentDirectionNPC1 = math.random(0, 3)
            self.directionTimer = 0
        end

        if Entity.CanMove(self.npcID1, self.currentDirectionNPC1) then
            Entity.Move(self.npcID1, self.currentDirectionNPC1)
        else
            if self.currentDirectionNPC1 == Direction.North then
                self.currentDirectionNPC1 = Direction.South
            
            elseif self.currentDirectionNPC1 == Direction.South then
                self.currentDirectionNPC1 = Direction.North

            elseif self.currentDirectionNPC1 == Direction.East then
                self.currentDirectionNPC1 = Direction.West
            
            else
                self.currentDirectionNPC1 = Direction.East

            end
        end
    end
end