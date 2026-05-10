function init(self)
    self.tvEntityID = -1
    self.mumEntityID = -1

    self.onActivate = function(self)
        self.tvEntityID = Entity.Create(SimpleVector.new(2, 2), SimpleVector.new(2, 1))
        Entity.SetActive(self.tvEntityID, true)
        Entity.AddDialogueComponent(self.tvEntityID, Utility.Hash("GENERIC_TV_1"), true, 10)

        self.mumEntityID = Entity.Create(SimpleVector.new(8, 7), SimpleVector.new(1, 1))
        Entity.SetActive(self.mumEntityID, true)
        Entity.AddAnimationComponent(self.mumEntityID, "PLAYER_MUM", AnimationName.IDLE_DOWN)
        Entity.AddScriptComponent(self.mumEntityID, "../scripts/entities/move_in_square.lua")
        Entity.SetScriptValue(self.mumEntityID, "walkDuration", "1")
        Entity.SetScriptValue(self.mumEntityID, "pauseDuration", "5")


        Entity.AddGridMovementComponent(self.mumEntityID, false)

        --TODO: Think of a way to make the TVs select random dialogue from the list
        --TODO: I think we need the concept of a "one-shot" dialogue line instead of going through the XML
        Entity.AddDialogueComponent(self.mumEntityID, Utility.Hash("PLAYER_MUM"), true, 0)
    end

    self.onDeactivate = function(self)
        Entity.Destroy(self.tvEntityID)
        Entity.Destroy(self.mumEntityID)

        self.tvEntityID = -1
        self.mumEntityID = -1
    end

    self.update = function(self, dt)
    end
end
