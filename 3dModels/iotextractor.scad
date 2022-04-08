use <../libs/openscad-rpi-library/misc_boards.scad>

/**
include <../libs/NopSCADlib/core.scad>
include <../libs/NopSCADlib/vitamins/fan.scad>
use <../libs/NopSCADlib/utils/layout.scad>
**/
include <../libs/NopSCADlib/lib.scad>  



THICK=2;
M5=5.5;
M4=4.5;
M3=3.5;
M2=2.5;
M3INSERT=3.7;
M2INSERT=3.4;

fanX=81;
fanY=81;
fanZ=26;
fanFix=35.7;

mountDep=5;


oledX=12;
oledY=38;
oledZ=3.5;
oledXD=5;

oledHouseX=oledX+THICK+1;
oledHouseY=oledY+THICK+1 + M2*4;
oledHouseZ=oledZ + 6 + THICK;


rotEncX=12;
rotEncY=38;
rotEncZ=3.5;
rotEncXD=5;

rotEncHouseX=rotEncX+THICK+1;
rotEncHouseY=rotEncY+THICK+1 + M2*4;
rotEncHouseZ=rotEncZ + 6 + THICK +5;

bracketW=30;
bracketD=20;
bracketL=80;
pcbPitch=2.54;

//fan_guard(fan80x25, name = false, thickness = THICK*2);
//translate([0,0,3])
//    oled();

fans();
bracket();

translate([-41,0,0]){
    rotate([0,0,90])
        guard();
}
translate([-41,50,(oledHouseZ+THICK)/2]){
    rotate([0,0,90])
        rotEncHouse();
}



translate([41,0,0]){
    rotate([0,0,90])
        guard();
}
translate([41,50,(oledHouseZ+THICK)/2]){
    rotate([0,0,90])
        oledHouse();
}



translate([50,0,(oledHouseZ+THICK)/2]){
//   oledHouse();
//    rotEncHouse();
}

module oled(){
    color("blue"){
        translate([0,0,(oledZ)/2])
            cube([oledX, oledY-(oledXD*2), 1], center=true);
        cube([oledX, oledY, oledZ-1], center=true);
        translate([0,(oledY-2.5)/2,5/(-2)])
            cube([oledX, 2.5, 5], center=true);
        translate([0,oledY/2-9,-5])
            cube([oledX,18,1], center=true);
    }
}


module oledHouse(){
    difference(){
        cube([oledHouseX, oledHouseY, oledHouseZ], center=true);
        cube([oledX- 2, oledY- (oledXD*2), oledHouseZ*2], center=true);
        translate([0, 0, THICK/(-2)])
            cube([oledX+1, oledY+1, oledHouseZ], center=true);
        
        for (i=[-1,1]){ 
            translate([0, (oledY/2 + M3)*i, 0]){
                cylinder(d=M2, h=oledHouseZ*2, center=true);
                translate([0, 0, 4])
                    cylinder(d=M4, h=oledHouseZ, center=true);
            }
        }
        
        translate([-6,0,oledHouseZ/(-2)]){
            cube([12,12,THICK*2],center=true);
        }
    }
}



module fans(){
    translate([0,0,25/(-2)]){
        translate([-41,0,0])
            fan(fan80x25);
        translate([41,0,0])
            fan(fan80x25);
    }
    
    //translate([0,0,50])
    //fan_guard(fan80x25, name = false, thickness = THICK);
}


module t(){
    difference(){
        translate([0,0,3/2])
            cylinder(r=2.5,h=3,center=true);
        rotate_extrude(convexity = 10)
            translate([2, 0, 0])
                circle(r = 1, $fn = 100);
        
    }
}


module guard(){
    fan_guard(fan80x25, name = false, thickness = THICK);
    
    resize(newsize=[80,80,10])
        blade();
    resize(newsize=[50,50,10])
        blade();
    resize(newsize=[28,28,10])
        blade();
    
    resize(newsize=[16,16,10])
        blade();
    
    
    intersection(){
        translate([0,0,THICK/2*5]){
            for (a=[0,45,135]){
                rotate([0,0,a])
                    cube([THICK, 82, THICK*5], center = true);
            }
            translate([-5,0,0]){
                rotate([0,0,90])
                    cube([THICK, 82, THICK*5], center = true);
            }
        }
        scale([1,1,0.5])
                sphere(d=82);
    }
    
    translate([48,0,THICK]){
        difference(){
            cube([oledHouseX+5, oledHouseY, THICK*2], center=true);
            translate([-5,0,THICK])
                cube([oledHouseX+5, oledHouseY-12, THICK*2], center=true);
            for (i=[-1,1]){ 
            translate([1, (oledY/2 + M3)*i, 0]){
                cylinder(d=M2INSERT, h=oledHouseZ*2, center=true);
            }
        }
        }
    }
}



module blade(){
    rotate_extrude(convexity = 10)
            translate([2, 0, 0])
                intersection(){
                    difference(){
                        circle(r = 1, $fn = 100);
                        circle(r = 0.9, $fn = 100);
                    }
                    translate([-1,1,1])
                        square([2,2],center=true);
                };
}




module rotEncHouse(){
    difference(){
        cube([rotEncHouseX+3, rotEncHouseY, rotEncHouseZ], center=true);
        translate([pcbPitch/2,0,0])
            cylinder(d=7.5, h=rotEncHouseZ*2, center=true);
        translate([0, 0, THICK/(-2)])
            cube([rotEncX+3, rotEncY+1, rotEncHouseZ], center=true);
        
        for (i=[-1,1]){ 
            translate([0, (rotEncY/2 + M3)*i, 0]){
                cylinder(d=M2, h=rotEncHouseZ*2, center=true);
                translate([0, 0, 4])
                    cylinder(d=M4, h=rotEncHouseZ, center=true);
            }
        }
        
        translate([-6,0,rotEncHouseZ/(-2)]){
            cube([12,12,THICK*2],center=true);
        }
    }
}


module bracket(){
    translate([41,46,(bracketD/-2)+THICK]){
        difference(){
            cube([bracketL, bracketW, bracketD], center=true); 
            
            //Front 90Deg
            translate([0,-10,THICK*(-1)]){
                cube([bracketL*1.5, bracketW/2, bracketD], center=true);
            }
            
            //Back 90Deg
            translate([0,10,THICK*(1)]){
                cube([bracketL*1.5, bracketW/2+THICK*3, bracketD], center=true);
            }
            
            //PCB Mount
            for (i=[0:pcbPitch*4:40]){
                for (j=[-1,1]){
                    translate([0+i*j,8,0]){
                        cylinder(d=M2, h=bracketD*2, center=true);
                    }
                }
            }
            
            //Fan Fixing
            for (i=[1,-1]){
                translate([36*i,-10,0]){
                    cylinder(d=M4, h=bracketD*2, center=true);
                }
            }
        }
        
    }
}