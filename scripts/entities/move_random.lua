function init(self)
    self.directionTimer = 0
    self.moving = true
    self.direction = Direction.North
    self.maxRandomDuration = 3
    self.randomDuration = self.maxRandomDuration

    self.onCreated = function(self)
        self:changeDirection()
    end

    self.onDestroyed = function(self)
    end

    self.onActivate = function(self)
        self:changeDirection()
    end

    self.onDeactivate = function(self)
    end

    self.onPlayerInteract = function(self)
        if not Entity.CanEntityBeInteractedWith(self.LOCAL_ENTITY_ID) then
            return
        end

        -- TODO: I need to make a way to get the player ID from the scripts...
        Entity.TurnToFace(self.LOCAL_ENTITY_ID, 1)

        self.moving = not Entity.HasDialogueLeft(self.LOCAL_ENTITY_ID) and not Dialogue.IsVisible()
    end

    self.changeDirection = function(self)
        if self.direction == Direction.North then
            self.direction = Direction.East
        elseif self.direction == Direction.South then
            self.direction = Direction.West
        elseif self.direction == Direction.East then
            self.direction = Direction.South
        elseif self.direction == Direction.West then
            self.direction = Direction.North
        end
    end

    self.update = function(self, dt)
        if self.moving == false then
            return
        end

        self.directionTimer = self.directionTimer + dt
        if self.directionTimer > self.randomDuration then
            self:changeDirection()

            self.randomDuration = math.random(1, self.maxRandomDuration)
            self.directionTimer = 0
        end

        if Entity.CanMove(self.LOCAL_ENTITY_ID, self.direction) then
            Entity.Move(self.LOCAL_ENTITY_ID, self.direction)
        end
    end
end
