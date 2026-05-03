function init(self)
    self.directionTimer = 0
    self.moving = true
    self.direction = Direction.North
    self.randomDuration = 1

    self.onCreated = function(self)
        print("Constructed! EntityID="..self.LOCAL_ENTITY_ID)
    end

    self.onDestroyed = function(self)
        print("Destroyed!")
    end

    self.onActivate = function(self)
        print("Owning entity set to active!")
    end

    self.onDeactivate = function(self)
        print("Owning entity set to unactive!")
        self.firstUpdateSinceActive = true;
    end

    self.onPlayerInteract = function(self)
        -- TODO: I need to make a way to get the player ID from the scripts... 
        Entity.TurnToFace(self.LOCAL_ENTITY_ID, 1)

        self.moving = not Entity.HasDialogueLeft(self.LOCAL_ENTITY_ID) and not Dialogue.IsVisible()
    end

    self.update = function(self, dt)
        if self.moving == false then
            return
        end

        self.directionTimer = self.directionTimer + dt
        if self.directionTimer > self.randomDuration then
            if Entity.CanMove(self.LOCAL_ENTITY_ID, self.direction) then
                Entity.Move(self.LOCAL_ENTITY_ID, self.direction)
            end

            if self.direction == Direction.North then
                self.direction = Direction.East

            elseif self.direction == Direction.East then
                self.direction = Direction.South

            elseif self.direction == Direction.South then
                self.direction = Direction.West

            else
                self.direction = Direction.North
            end
        
            self.randomDuration = math.random(1, 5)
            self.directionTimer = 0
        end    
    end
end