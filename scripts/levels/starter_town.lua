function init(self)
    self.man1 = -1
    self.lampPost1 = -1
    self.cyclist1 = -1
    self.fisherman1 = -1

    --- func desc
    ---@param xPosition number
    ---@param yPosition number
    ---@param animationConfig { dictName: string, initialAnim: number } | nil
    ---@param dialogueConfig { dialogueID: string, isLooping: boolean | nil, cooldownTime: number | nil } | nil
    ---@param scriptName string | nil
    self.spawnNPC = function(self, xPosition, yPosition, animationConfig, dialogueConfig, scriptName)
        print("spawnNPC")

        local id = Entity.Create(xPosition, yPosition)

        print("Created entity with ID"..id)

        Entity.SetActive(id, true)

        if animationConfig ~= nil then
            print("Adding animation to Entity "..id.." with the dict="..animationConfig.dictName.." and initial anim "..animationConfig.initialAnim)
            local isCycling = animationConfig.initialAnim & 0xF000 > 0
            print("isCycling="..tostring(isCycling))
            Entity.AddGridMovementComponent(id, isCycling)
            Entity.AddAnimationComponent(id, animationConfig.dictName, animationConfig.initialAnim)
        end

        if dialogueConfig ~= nil then
            local dialogueID = dialogueConfig.dialogueID
            local isLooping = false
            if dialogueConfig.isLooping ~= nil then
                isLooping = dialogueConfig.isLooping
            end

            local cooldownTime = 3
            if dialogueConfig.cooldownTime ~= nil then
                cooldownTime = dialogueConfig.cooldownTime
            end


            Entity.AddDialogueComponent(id, dialogueID, isLooping, cooldownTime)
        end

        if scriptName ~= nil then
            Entity.AddScriptComponent(id, "../scripts/entities/"..scriptName..".lua")
        end

        return id;
    end

    self.onActivate = function(self)
        print("onActivate")

        print("Entity IDs=".." "..self.man1.." "..self.lampPost1.." "..self.cyclist1.." "..self.fisherman1.." "..self.monster1.." "..self.monster2.." "..self.child1)
        self.man1 = self:spawnNPC(
            23 * 32,
            15 * 32,
            {
                dictName="NPC_MAN_1",
                initialAnim=AnimationName.IDLE_DOWN
            },
            {
                dialogueID="STARTER_TOWN_MAN_1",
                isLooping=false
            },
            "test_script_1"
        )

        self.lampPost1 = self:spawnNPC(
            17 * 32,
            28 * 32,
            nil,
            {
                dialogueID="STARTER_TOWN_PROFESSOR_SIGN",
                cooldownTime=0,
                isLooping=true
            },
            nil
        )

        self.cyclist1 = self:spawnNPC(
            28 * 32,
            20 * 32,
            {
                dictName="FEMALE_CYCLIST",
                initialAnim=AnimationName.CYCLE_DOWN
            },
            {
                dialogueID="STARTER_TOWN_CYCLIST",
                cooldownTime=5,
                isLooping=true
            },
            "move_in_square"
        )

        Entity.SetScriptValue(self.cyclist1, "squareWidth", "7")
        Entity.SetScriptValue(self.cyclist1, "squareHeight", "8")

        self.fisherman1 = self:spawnNPC(
            31 * 32,
            22 * 32,
            {
                dictName="MALE_FISHERMAN",
                initialAnim=AnimationName.IDLE_DOWN
            },
            {
                dialogueID="STARTER_TOWN_FISHERMAN",
                cooldownTime=10,
                isLooping=true
            },
            nil
        )
    end

    self.onDeactivate = function(self)
        print("onDeactivate starter_town")

        Entity.Destroy(self.man1)
        Entity.Destroy(self.lampPost1)
        Entity.Destroy(self.cyclist1)
        Entity.Destroy(self.fisherman1)

        self.man1 = -1
        self.lampPost1 = -1
        self.cyclist1 = -1
        self.fisherman1 = -1
    end

    self.update = function(self, dt)
        
    end
end
