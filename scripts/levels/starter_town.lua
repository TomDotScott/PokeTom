function init(self)
    self.man1 = -1
    self.lampPost1 = -1

    --- func desc
    ---@param xPosition number
    ---@param yPosition number
    ---@param animDict string | nil
    ---@param dialogueID string | nil
    ---@param scriptName string | nil
    self.spawnNPC = function(self, xPosition, yPosition, animDict, dialogueID, scriptName)
        print("spawnNPC")

        local id = Entity.Create(xPosition, yPosition)

        print("Created entity with ID"..id)

        Entity.SetActive(id, true)

        if animDict ~= nil then
            Entity.AddGridMovementComponent(id)
            Entity.AddAnimationComponent(id, "animation/"..animDict..".xml", AnimationName.IDLE_DOWN)
        end

        if dialogueID ~= nil then
            Entity.AddDialogueComponent(id, dialogueID, true, 3)
        end

        if scriptName ~= nil then
            Entity.AddScriptComponent(id, "../scripts/entities/"..scriptName..".lua")
        end

        return id;
    end

    self.onActivate = function(self)
        print("onActivate")


        self.man1 = self:spawnNPC(23 * 32, 15 * 32, "NPC_MAN_1", "STARTER_TOWN_MAN_1", "test_script_1")
        self.lampPost1 = self:spawnNPC(17 * 32, 28 * 32, nil, "STARTER_TOWN_PROFESSOR_SIGN", nil)
    end

    self.onDeactivate = function(self)
        print("onDeactivate starter_town")

        Entity.Destroy(self.man1)
        Entity.Destroy(self.lampPost1)

        self.man1 = -1
        self.lampPost1 = -1
    end

    self.update = function(self, dt)
        
    end
end
