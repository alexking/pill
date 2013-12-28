/*
 * PillBug
 * by Alex King (alexking.io)
 *
 * This is free and unencumbered software released into the public domain.
 * Visit http://unlicense.org for the full license.
 *
 */

metadata
{

    tiles 
    {
    
        standardTile("switch", "device.switch", width : 2, height : 2)
        {
            state "on", label : "Take Pills",  backgroundColor : "#FF0000"
            state "off", label : "✔", backgroundColor : "#79B821"
        }
    
        standardTile("contact", "device.contact", width : 1, height : 1)
        {
            state "open", label : "Gone"
            state "close", label : "Here"
        }
    
    
        standardTile("test", "device.switch", width : 1, height : 1)
        {
            state "on", label : "On",  action : "switch.off"
            state "off", label : "Off", action : "switch.on"
        }

        main(["switch"])
        details(["switch", "contact", "test"])
    }
}

def on() 
{
    // Pills need to be taken 
    zigbee.smartShield( text : "on" ).format()
}

def off()
{
    // Pills have been taken 
    zigbee.smartShield( text : "off" ).format()
}

def parse(String description)
{
    def event

    // Parse the description 
    def data = zigbee.parse(description)
        
    // Handle different commands 
    switch (data.text) 
    {
    
        // Pill Bottle Status 
        case "open":   
        case "close":
        
            event = [
                name            : "contact",
                value           : data.text,
                isStateChange   : true, 
                isPhysical      : true, 
                displayName     : device.displayName,
                displayed       : true,
                descriptionText : data.text == "open" ? "Pill bottle removed" : "Pill bottle present",
                source          : "DEVICE"
            ] 
                        
            // If this is a close event, and the switch is on, then 
            // this also triggers an "off" event
            if (data.text == "close" && device.currentValue("switch") == "on")
            {
                def offEvent = [
                    name            : "switch",
                    value           : "off",
                    isStateChange   : true, 
                    displayName     : device.displayName,
                    displayed       : true,
                    descriptionText : "Pills taken"
                ] 
                
                event = [event, offEvent]
            }
            
            break
        
        // On means take pills  
        case "on":
        case "off":
                       
            event = [
                name            : "switch",
                value           : data.text,
                isStateChange   : true, 
                displayName     : device.displayName,
                displayed       : true,
                descriptionText : data.text == "on" ? "Please take your pills" : "Pills taken"
            ] 
            
            
            break
        
        case "update":
            
            log.debug "Update requested..."
            def state = device.currentState("switch")
            def now = new Date() 
    
            def parameters = [ 
                "switch"  : state.value,
                "ago"     : Math.round((now.time - state.date.time) / (1000 * 60) ),
                "contact" : device.currentValue("contact")
                
            ]
    
            event = [ new physicalgraph.device.HubAction(zigbee.smartShield( 
                text : "set:" + parameters.collect{ k, v -> "${k}=${v}" }.join(",")
            ).format()) ]

            break 

        case "ping":
            // Return early 
            return 
    }
    
    // Log 
    log.debug "Event  ${event}"
        
    // Return the event 
    event
}


