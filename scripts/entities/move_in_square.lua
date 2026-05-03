function init(self)
    self.initialPosition = {x=0, y=0}

    -- Strings because the width and height can be changed by the C++ bindings
    self.squareWidth="1"
    self.squareHeight="1"

    self.shouldTurnToFacePlayer = true
    self.moving = true

    self.currentDirection = Direction.East

    self.onCreated = function(self)
        print("Constructed! EntityID="..self.LOCAL_ENTITY_ID)
        self.initialPosition = Entity.GetPosition(self.LOCAL_ENTITY_ID)
    end

    self.onDestroyed = function(self)
        print("Destroyed! "..self.LOCAL_ENTITY_ID)
    end

    self.onActivate = function(self)
        print("Owning entity set to active! "..self.LOCAL_ENTITY_ID)
    end

    self.onDeactivate = function(self)
        print("Owning entity set to unactive! "..self.LOCAL_ENTITY_ID)
    end

    self.onPlayerInteract = function(self)
        if self.shouldTurnToFacePlayer then
            -- TODO: I need to make a way to get the player ID from the scripts... 
            Entity.TurnToFace(self.LOCAL_ENTITY_ID, 1)
        end

        self.moving = not Entity.HasDialogueLeft(self.LOCAL_ENTITY_ID) and not Dialogue.IsVisible()
    end

    self.update = function(self, dt)
        if self.moving == false then
            return
        end
        
        --@type {x= number, y= number}
        local currentPosition = Entity.GetPosition(self.LOCAL_ENTITY_ID)

        local xDist = (currentPosition.x - self.initialPosition.x) / 32
        local yDist = (currentPosition.y - self.initialPosition.y) / 32

        -- ------>|
        -- ^      |
        -- |      |
        -- |      |
        -- <------v
        if self.currentDirection == Direction.North then
            if yDist <= 1 then
                self.currentDirection = Direction.East
            end
            
        elseif self.currentDirection == Direction.South then
            
            if yDist >= tonumber(self.squareHeight) - 1 then
                self.currentDirection = Direction.West
            end

        elseif self.currentDirection == Direction.East then

            if xDist >= tonumber(self.squareWidth) - 1 then
                self.currentDirection = Direction.South
            end

        elseif self.currentDirection == Direction.West then
            if xDist <= 1 then
                self.currentDirection = Direction.North
            end
        end

        if Entity.CanMove(self.LOCAL_ENTITY_ID, self.currentDirection) then
            Entity.Move(self.LOCAL_ENTITY_ID, self.currentDirection)
        end
    end
end