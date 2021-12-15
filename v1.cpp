#include "usercode.h"

#include <ipc_format.h>
#include <math.h>
#include <bits/stdc++.h>

using namespace std;

//input: ratio is between 0.0 to 1.0
//output: rgb color
IpcColor rgb(double ratio)
{
    //we want to normalize ratio so that it fits in to 6 regions
    //where each region is 256 units long
    int normalized = int(ratio * 256 * 6);

    //find the region for this position
    int region = normalized / 256;

    //find the distance to the start of the closest region
    int x = normalized % 256;

    uint8_t r = 0, g = 0, b = 0;
    switch (region)
    {
        case 0: r = 255; g = 0;   b = 0;   g += x; break;
        case 1: r = 255; g = 255; b = 0;   r -= x; break;
        case 2: r = 0;   g = 255; b = 0;   b += x; break;
        case 3: r = 0;   g = 255; b = 255; g -= x; break;
        case 4: r = 0;   g = 0;   b = 255; r += x; break;
        case 5: r = 255; g = 0;   b = 255; b -= x; break;
    }
    
    return IpcColor{r, g, b};
}

bool init(Api *api)
{
    api->clearColors();
    IpcColor color;
    for (double i = 0; i <= 1; i+= 0.0075) {
        color = rgb(i);
        api->addColor(color.r, color.g, color.b);
    }

    return true;
}

bool step(Api *api)
{
    // As a default we dont want to boost
    api->boost = false;
    
    // Detect enemies and avoid them
    bool enemyDetected = false;
    double movingX = 0;
    double movingY = 0;
    
    for(size_t i = 0; i < api->getSegmentCount(); i++) {
        const IpcSegmentInfo &segment = api->getSegments()[i];
        
        // TODO: Skip own segments or enemies that are to far away
        if (segment.is_self || segment.dist > 150)
            continue;
     
        // Draw a vector to the opposite of the enemy
        movingX -= sin(segment.dir * (1 / segment.dist));
        movingY -= cos(segment.dir * (1 / segment.dist));
        enemyDetected = true;
    }


    // If there are any enemies avoid them
    if (enemyDetected) {
        long resVectorLength = sqrt((movingX * movingX) + (movingY * movingY));
        long resVectorDir = atan2(movingX, movingY);
        
        void* persistentMemory = api->getPersistentMemory();
        *((int*)persistentMemory) = (int)resVectorLength;
        
        // If a enemy is crazy near boost away from it
        if (resVectorLength > 20) {
            api->boost = true;
        }
        
        // TODO: If the length of the resulting vector is larger than a threshold (TBD) -> boost
        //stringstream message;
        //message << "length of resulting vector: " << resVectorLength;
        //api->log(message.str().c_str());
        
        
        api->angle = resVectorDir;
        return true;
    }
    
    
    // Eat food
    
    // If there is no food, keep going forward
    if (api->getFoodCount() == 0) {
        api->angle = 0;
        return true;
    }
    
    // Run to next food
    const IpcFoodInfo &seg = api->getFood()[0];
    api->angle = seg.dir;
    
    return true;
}
