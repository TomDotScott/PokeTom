function init(self)
    self.npcID1 = 0
    self.currentDirectionNPC1 = Direction.North

    self.onActivate = function(self)
        print("onActivate")

        self.npcID1 = Entity.Create(736.0, 384.0)

        Entity.SetActive(self.npcID1, true)

        print(self.npcID1)

        Entity.AddAnimationComponent(self.npcID1, "animation/npc_man_1.xml", AnimationName.IDLE_DOWN)
        Entity.AddGridMovementComponent(self.npcID1)
    end

    self.onDeactivate = function(self)
        print("onDeactivate")
        Entity.Destroy(self.npcID1)
    end

    self.update = function(self, dt)
        if Entity.CanMove(self.npcID1, self.currentDirectionNPC1) then
            Entity.Move(self.npcID1, self.currentDirectionNPC1)
        else
            if self.currentDirectionNPC1 == Direction.North then
                self.currentDirectionNPC1 = Direction.South
            else
                self.currentDirectionNPC1 = Direction.North
            end
        end
    end
end