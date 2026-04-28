function init(self)
    self.manID = -1

    self.spawnNPC = function(self, x, y)
        if self.manID ~= -1 then
            Entity.Destroy(self.manID)
        end

        print("spawnNPC")

        self.manID = Entity.Create(x, y)

        print("Created entity with ID" .. self.manID)

        Entity.SetActive(self.manID, true)


        Entity.AddAnimationComponent(self.manID, "animation/npc_man_1.xml", AnimationName.IDLE_DOWN)
        Entity.AddGridMovementComponent(self.manID)
        Entity.AddDialogueComponent(self.manID, "STARTER_TOWN_MAN_1", true, 3)
        Entity.AddScriptComponent(self.manID, "../scripts/entities/test_script_1.lua")

        print(self.manID)
    end

    self.onActivate = function(self)
        print("onActivate")

        self:spawnNPC(736.0, 384.0)
    end

    self.onDeactivate = function(self)
        print("onDeactivate")

        Entity.Destroy(self.manID)
    end

    self.update = function(self, dt)
        
    end
end
