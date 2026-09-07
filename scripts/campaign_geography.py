"""Compile eight authored overworld layouts; no random encounters or runtime path generation."""
import math

def tiles(region, number):
    sites=region['sites']; grid=[[0 for _ in range(64)] for _ in range(48)]
    def oval(x,y,cx,cy,rx,ry):return ((x-cx)/rx)**2+((y-cy)/ry)**2<1
    for y in range(48):
        for x in range(64):
            water=False;rock=False
            if number==0:
                water=abs(x-(24+int(3*math.sin(y/7))))<=1 or oval(x,y,12,9,5,3)
            elif number==1:
                water=not any(oval(x,y,*o) for o in [(10,26,11,12),(29,35,14,10),(46,30,15,12),(32,13,12,9),(51,14,10,10)])
            elif number==2:
                rock=(y in range(11,14) or y in range(24,27) or y in range(36,39)) and x>14
                water=x>59 or (x>52 and y>39)
            elif number==3:
                water=oval(x,y,34,27,6,5) or (y<5 and x>25)
                rock=x%12==6 and 12<y<38 and y%10>2
            elif number==4:
                water=oval(x,y,35,25,9,8) or (x>58 and y>30) or (y>42 and x>25)
                rock=oval(x,y,35,25,12,11) and not water
            elif number==5:
                water=oval(x,y,31,24,12,6) or (x>51 and y<13)
                rock= y in (17,33) and 18<x<51
            elif number==6:
                water=x>50-y//4 or (x>36 and y>32)
                water=water and not oval(x,y,53,16,9,9) and not oval(x,y,51,36,9,9)
            else:
                water=not any(oval(x,y,*o) for o in [(9,26,8,11),(23,10,13,8),(46,13,14,11),(48,33,13,12),(24,37,13,9),(32,21,8,8)])
            grid[y][x]=2 if water else 3 if rock else 0
    # Each region follows a different authored journey. Side paths branch toward
    # visible habitats, caches and optional trials. Bridges preserve every route.
    road=region['road'];edges=list(zip(road,road[1:]))
    for i,s in enumerate(sites):
        if i not in road:
            closest=min(road,key=lambda j:abs(s['x']-sites[j]['x'])+abs(s['y']-sites[j]['y']))
            edges.append((i,closest))
    for a,b in edges:
        x,y=sites[a]['x'],sites[a]['y'];tx,ty=sites[b]['x'],sites[b]['y']
        axes=(0,1) if (a+number)%2 else (1,0)
        for axis in axes:
            while (x!=tx if axis==0 else y!=ty):
                grid[y][x]=4 if grid[y][x]==2 else 1
                if axis==0:x+=1 if tx>x else -1
                else:y+=1 if ty>y else -1
            grid[y][x]=4 if grid[y][x]==2 else 1
    for s in sites:
        for y in range(s['y']-1,s['y']+2):
            for x in range(s['x']-1,s['x']+2):grid[y][x]=1
    # Sanctuary approach is stable across chapters, saves and defeat recovery.
    for y in range(23,29):
        for x in range(6,11):grid[y][x]=1
    for x in range(64):grid[0][x]=grid[47][x]=2
    for y in range(48):grid[y][0]=grid[y][63]=2
    return grid
