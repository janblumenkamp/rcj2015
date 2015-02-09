////////////////Sharp IR////////////////////////////////////////////
//////Shortrange/////////

module sharp_IR_short(raylength, shield_on)
{
	translate([0,0,1.75/2])
	{
		color("grey")
		{
			difference()
			{
				rotate(0)
				{
					cube([37,7.5,1.75],true);
					translate([18.5,0,0])
					cylinder(h=1.75,r=3.75, center = true);
					translate([-18.5,0,0])
					cylinder(h=1.75,r=3.75, center = true);
				}
				translate([18.5,0,0])
				cylinder(h=1.9,r=1.6, center = true);
				translate([-18.5,0,0], center = true)
				cylinder(h=1.9,r=1.6, center = true);
			}
			translate([0,0,4.875])
			cube([29.5,8.4,11.5],true);
			translate([0,0,2.625])
			cube([29.5,13,7],true);

			translate([-10.55,0,11.5])
			difference()
			{
				cube([7.2,7.2,2],true);
				translate([0,0,2])
				{
					rotate([180,0,0])
					{
						cylinder(10,3.2);
					}
				}
			}
			translate([-10.55,0,10])
			sphere(r = 3);
			translate([6,0,11.5])
			difference()
			{
				cube([16.3,7.2,2],true);
				cube([15.5,6.4,2.5],true);
			}
			translate([10.55,0,9])
			{
				sphere(r = 3.3);
				if(shield_on)
				{
					difference()
					{
						cylinder(h=35,r=3);
						cylinder(h=36,r=2.5);
					}
				}
			}
		}
		
		translate([10.55,0,0])
		#%color("white")
		cylinder(h=raylength,r=2.5);
		
		color("green")
		translate([0,-9.45,0.6])
		cube([10.1,5.9,1.2],true);
		
		color("white")
		translate([0,-9.75,3.2])
		cube([8.5,6.5,4],true);
	}
}


/////////Longrange////IR
module sharp_IR_long()
{
	rotate([270,0,0])
	{
		translate([42.1,-24.075,-19.73])
		{
			color("grey")
			import("Sharp_IR_Long_Binary.stl");
		}
	}
}

/////////Neuer Sensor (4-50cm)////IR
module sharp_IR_new(raylength)
{
	translate([0,-4,1])
	{
		color("green")
		cube([11,16.7,1],true);

		color("black")
		translate([0,2,2])
		cube([10,6,4.2],true);
		
		#%color("white")
		translate([-2,2,5])
		cylinder(h=raylength, r=2);
		
		color("dimgray")
		translate([0,-6,1])
		cube([9,4,2],true);
	}
}

/////////////////SRF10//////////
module srf10(fov_on)
{
	color("green")
	cube([32,15,1.2],true);
	color("silver")
	{
		translate([-9,0,0])
		cylinder(h=6.5,r=4.9);
		translate([9,0,0])
		cylinder(h=6.5,r=4.9);
	}
	
	if(fov_on)
	{
		#%color("white")
		translate([10,0,300])
		rotate([180,0,0])
		cylinder(300,170);
	}
}
//////////Platine//////////
module pcb_main(rnmega_on, pololu_on)
{
	color("green") //Motherboard
	difference()
	{
		cube([100,80,1.6],true);

		translate([46,0,-1.5])    //Löcher
		cylinder(h=3,r=drill_m3/2);
		translate([46,-36,-1.5])    //Löcher
		cylinder(h=3,r=drill_m3/2);
		translate([1,12,-1.5])    //Löcher
		cylinder(h=3,r=drill_m3/2);
		translate([-42,-36,-1.5])    //Löcher
		cylinder(h=3,r=drill_m3/2);
		translate([-42,36,-1.5])    //Löcher
		cylinder(h=3,r=drill_m3/2);
	}
		color("black") //Buchsenleisten
		{
			translate([-63/2,-43/2,0])
			cube([63,2,8]);
			translate([-63/2,43/2-2,])
			cube([63,2,8]);
		}
		color("lightgrey") //NXT Buchsen
		{
			translate([-40,-43/2,7])
			cube([15,12,14.5],true);
			translate([-40,43/2-2,7])
			cube([15,12,14.5],true);
		}
		color("white") //LED
		{
			translate([20,33,1.5])
			cube([5,5,2],true);	
		}

	if(rnmega_on)
	{
		color("green")		//RNmega
		translate([0,0,8])
		cube([63,43,1.6],true);
			rotate([0,0,45])
			translate([0,0,9])
			color("black")
			cube([14,14,1.2],true); //ATmega2560
	}
	
	if(pololu_on)
	{
		translate([-41,0,8])
		color("green")
		cube([15.24,20.32,1.6],true); //Pololu Motordriver
			
			color("black")
			translate([-41,0,9])
			cube([5.6,7.8,1.2],true); //IC
	}
	color("black")
	{
		translate([-41-(15.24/2)+1,0,3.5]) //Buchsenleisten Pololu
		cube([2,20.32,8],true);
		translate([-41+(15.24/2)-1,0,3.5])
		cube([2,20.32,8],true);
	}
}

////Powerunit///
module powerunit()
{
	color("darkkhaki")
	difference()
	{
		cube([40,70,1.6],true);

		translate([17.5,32.5,-1])
		cylinder(h = 2, r = 1.5);
		translate([-17.5,32.5,-1])
		cylinder(h = 2, r = 1.5);
		translate([17.5,-32.5,-1])
		cylinder(h = 2, r = 1.5);
		translate([-17.5,-32.5,-1])
		cylinder(h = 2, r = 1.5);
	}
	color("lightgrey") //NXT Buchsen
	{
		translate([12,7,7])
		cube([15,12,14.5],true);
		translate([12,23,7])
		cube([15,12,14.5],true);
	}
	color("black") //Wannenstecker
	{
		translate([-2,22,2.5])
		cube([4,9,4],true);
		translate([-12,22,2.5])
		cube([4,9,4],true);
		translate([-2,6,2.5])
		cube([4,9,4],true);
		translate([-12,6,2.5])
		cube([4,9,4],true);
	}
	color("darkslategrey")
	{
		translate([14,-8,4.5])
		cube([12,10.16,8],true);
		translate([25,-8,8])
		cube([10,10.16,15.5],true);

		translate([-14,-10,4.5])
		cube([12,10.16,8],true);
		translate([-25,-10,8])
		cube([10,10.16,15.5],true);

		translate([-14,-25,4.5])
		cube([12,10.16,8],true);
		translate([-25,-25,8])
		cube([10,10.16,15.5],true);
	}
}

////////LCD/////////

module lcd()
{
	color("green") //Platine
	difference()
	{
		cube([70,93,1.2],true);
		translate([-32.5,-44,-1])
		cylinder(h=3,r=1.5);
		translate([32.5,-44,-1])
		cylinder(h=3,r=1.5);
		translate([-32.5,44,-1])
		cylinder(h=3,r=1.5);
		translate([32.5,44,-1])
		cylinder(h=3,r=1.5);
	}

	color("black")	//Buchsenleiste
	translate([-34,10,-1.6])
	cube([2,50,2],true);
	color("darkkhaki") //Adapterplatine
	translate([-26.5,10,-3])
	cube([15,50,1.6],true);

	color("black") //Rahmen
	difference()
	{
		translate([0,0,5])
		cube([51,78,9],true);
		translate([0,0,5])
		cube([40,72,9.1],true);
	}
	color("blue")	//LCD
	translate([0,0,5])
	cube([40,72,7.5],true);
}

///////UM6/////
module um6(pcb_on)
{
	color("chocolate") //"Motherboard"
	difference()
	{
		cube([45,25,1.2],true);
		translate([4.5,4,-1])
		cylinder(h=2,r=drill_m3/2);
	}	
	color("red") //Wannenstecker
	translate([-12.5,-8.5,-2.1])
	cube([10,4,3],true);
	
	color("black") //Buchsenleisten
	{
		translate([-14,10.5,4]) //USB
		cube([16,2,8],true);
		translate([-21,3.5,4])
		cube([2,12,8],true);
		translate([-7,3.5,4])
		cube([2,12,8],true);

		translate([7,10.5,4])
		cube([15,2,8],true);

		translate([10,-10.5,4])
		cube([24,2,8],true);
	}
	if(pcb_on)
	{
		color("green") //UM6
		translate([10,0,8.6])
		cube([25,24,1.2],true);
		color("black") //UM6 ICs
		{
			translate([4,0,9.8])
			cube([6,6,1.2],true);
			translate([16,-5,9.8])
			cube([3,3,1.2],true);
			translate([16,5,9.8])
			cube([4,4,1.2],true);
		}
		color("green") //USB
		translate([-14,0,8.6])
		cube([16,24,1.2],true);
		color("black") //USB IC
		translate([-13,4,9.8])
		cube([4,4,1.2],true);
		color("grey") //USB Conn.
		translate([-15,-8,11])
		cube([7,9,4],true);
	}
}

////////////////UIunit///////////////////
module uiunit(ledheight)
{
	color("darkkhaki")
	{
		difference()
		{
			cube([60,20,1.55],true);
			
			translate([-25,0,-1])
			cylinder(h = 2, r = 1.5);
			translate([25,0,-1])
			cylinder(h = 2, r = 1.5);
		}
	}
	translate([18,0,ledheight])
	led_5mm(ledheight);
	translate([-18,0,ledheight])
	led_5mm(ledheight);

	color("red") //Wannenstecker
	{
		translate([5,-2,-2.1])
		cube([10,4,3],true);

		translate([-9,-2,-2.1])
		cube([6,4,3],true);
	}
}
//////////ADCexpansion/////////////////
module adcexpansion()
{
	color("chocolate")
	{
		difference()
		{
			cube([20,100,1.55],true);

			translate([6,-46,-1])
			cylinder(h=2,r=1.5);
			translate([6,46,-1])
			cylinder(h=2,r=1.5);
		}
	}
	color("red") //Wannenstecker
	{
		translate([-3,-10,2.1])
		cube([4,10,3],true);
		translate([5,-30,2.1])
		cube([4,6,3],true);
		translate([5,0,2.1])
		cube([4,6,3],true);
		translate([5,30,2.1])
		cube([4,6,3],true);
	}
}
/////////////HP LED////////////////

num_cuts = 6; // number of circler cutouts
disc_w = 20; // LED heatsink width
disc_t = 1.5; // LED heatsink thickness
cut_dia = 3.6; // cutout circle dia
cut_r = 10.0; // distance from disc center to cutout center
LED_dia = 8; // LED dia
LED_h = 2.6;	// LED base height
LED_dome = 2.62; // dome height

module HP_LED()
{
	translate([0,0,disc_t])
	{
		color("black")
		cylinder(r=LED_dia/2,h=LED_h);
 		translate([0,0,LED_h])
		color("white")
		sphere(r=LED_dome);
	}
 	color("darkgrey")
	difference()
	{
  		for(i=[0:num_cuts-1])
		{
   			rotate(i*(360/num_cuts))
    		translate([0,0,disc_t/2])
			cube([disc_w,disc_w/2-cut_dia/2,disc_t],center=true);
  		}
  		for(i=[0:num_cuts-1])
		{ // circler cutouts
  			rotate(i*(360/num_cuts)+(360/num_cuts)/2)
    		translate([cut_r,0,0])
     		cylinder(r=cut_dia/2,h=disc_t*2+1,center=true);
  		}
 	}
}
//////////////////////Switch//////////////
module switch()
{
	color("darkgrey")
	{
		cube([19.2,12.5,10],true);
		translate([0,0,5])
		cube([21.2,14.5,1],true);
		translate([0,0,5])
		rotate([0,10,0])
		cube([16,9,5],true);
	}
}

//////////////Inkrementalgeber///////////////
module incremental()
{
	color("silver")
	{
		cube([11.7,12,4.5],true);
		translate([0,0,-4.25])
		cube([12,15.5,4],true);

		translate([0,0,2.25])
		cylinder(h = 7, r = 3.5);
	}
	color("darkgrey")
	{
		translate([0,0,9])
		cylinder(h = 14, r = 3);
	}
}

module incremental_btn()
{
	color("grey")
	translate([0,0,8])
	cylinder(h = 16, r = 6.5);
}
///////////////LED//////////
module led_3mm()
{
	color("white")
	{
		difference()
		{
			cylinder(h = 1, r = 2);
			translate([0,-2,0.5])
			cube([5,1,2],true);
		}
		cylinder(h = 5, r = 1.5);
		translate([0,0,5])
		sphere(r = 1.5);
	}
}

module led_5mm(leglength)
{
	color("white")
	{
		difference()
		{
			cylinder(h = 1, r = 3);
			translate([0,-3.5,0.5])
			cube([7,2,2],true);
		}
		cylinder(h = 5, r = 2.5);
		translate([0,0,5])
		sphere(r = 2.5);

		translate([0,-1.5,-leglength])
		cylinder(h = leglength, r = 0.3);
		translate([0,1.5,-leglength])
		cylinder(h = leglength, r = 0.3);
	}
}
////////////Taster//////////////
module pushbutton()
{
	color("black")
	{
		cylinder(h = 15.7, r = 3.5);
		translate([0,0,7])
		cylinder(h = 2, r = 5);
	}
	color("red")
	{
		translate([0,0,20])
		cylinder(h = 1, r = 3);
		translate([0,0,15.5])
		cylinder(h = 4.5, r = 1);
	}
}

////////////MLX90614///////////////
module mlx(rot_x, rot_y, rot_z, fov_angle)
{
	rotate([rot_x,rot_y,rot_z])
	translate([0,0,11])
	{
		color("chocolate")
		cube([15,1.55,23.5],true);

		color("black")
		translate([0,0,6.6])
		rotate([270,0,0])
		cylinder(h = 11, r = 5);

		color("red")			//Micromatch
		translate([0,-2.1,6])
		cube([4,3,10],true);

		if(fov_angle)
		{
			#%color("white")
			translate([0,153,6.6])
			rotate([90,0,0])
			cylinder(150,150*tan(fov_angle));
		}
	}
	color("firebrick")
	{
		translate([0,-8,0])
		difference()
		{
			cylinder(h = 2, r = 5);
			translate([0,0,-0.25])
			cylinder(h = 2.5, r = 2.5);
		}
		rotate([0,0,-40])
		translate([0,-6,0])
		cube([2,7,2]);
	}
}

//////////////CMUcam//////////////

module cmucam(fov_on)
{
	color("black") //Platine
	{
		cube([45,54,1.4],true);
		difference()
		{
			translate([0,-14.3,-0.7])
			cube([28.9,28.6,1.4]);
			translate([25.6,-10.4,-4.5])	//Löcher
			cylinder(h=9,r=1.5);
			translate([25.6,10.4,-4.5])
			cylinder(h=9,r=1.5);
		}
	}
	
	color("dimgray") //Objektiv
	{
		translate([-8,0,-6])
		cube([16,16,12],true);
		translate([-8,0,-25])
		cylinder(h=20, r=6);
	}
	
	if(fov_on)
	{
		#%color("white")
		translate([-8,0,-20])
		rotate([0,0,90])
		difference()
		{
			translate([-800,-800,-600])
			cube([1600,1600,600]);


			translate([0,-800,0])
			rotate([0,75/2,0])
			cube([1000,1600,1000]);
			translate([0,-800,0])
			rotate([0,-(90+75/2),0])
			cube([1000,1600,1000]);

			rotate([47,0,0])
			translate([-800,0,-1000])
			cube([1600,1000,1000]);

			rotate([-47,0,0])
			translate([-800,-1000,-1000])
			cube([1600,1000,1000]);
		}
	}
}

/////////Servo Micro
module servo_micro()
{
	translate([-17,0,0])
	{
		color("royalblue")
		{
			cube([22.3,11.9,19],true);
			difference()
			{
				translate([0,0,12.5])
				cube([32.2,11.9,2],true);

				translate([-14,0,10])
				cylinder(h=5,r=1.5);
				translate([14,0,10])
				cylinder(h=5,r=1.5);
			}
			translate([0,0,12.5])
			cube([22.3,11.9,6],true);
		}
		translate([-5,0,0])
		{
			color("black")
			cylinder(h=20.5,r=5.9);
			cylinder(h=24,r=4.55/2);
		}
	}
}
