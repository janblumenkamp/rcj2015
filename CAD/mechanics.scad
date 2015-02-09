///////////////////////Abgerundete Platte//////////////////////////

module createMeniscus(h,radius)
{
	difference()
	{
   		translate([radius/2+0.1,radius/2+0.1,0])
		{
      		cube([radius+0.2,radius+0.1,h+0.2],center=true);
   		}

   		cylinder(h=h+0.2,r=radius,center=true);
	}
}

module roundCornersCube(x,y,z,r)
{
	difference()
	{
   		cube([x,y,z], center=true);

		translate([x/2-r,y/2-r])
		{
      		rotate(0)
			{  
         		createMeniscus(z,r);
      		}
   		}
   		translate([-x/2+r,y/2-r])
		{
      		rotate(90)
			{
         		createMeniscus(z,r);
      		}
   		}
      	translate([-x/2+r,-y/2+r])
		{
      		rotate(180)
			{
         		createMeniscus(z,r);
      		}
   		}
      	translate([x/2-r,-y/2+r])
		{
     		rotate(270)
			{
         		createMeniscus(z,r);
      		}
   		}
	}
}

////////////////////////////Motor/////////////////////////////
spg30_encoderWidth = 15;
spg30_motorWidth = 30;
spg30_gearWidth = 22;

module spg30()
{
	color("black")
	cylinder(h=spg30_encoderWidth,r=15);		//Encoder
	color("lightgray")
	{
		translate([0,0,spg30_encoderWidth])
		cylinder(h=spg30_motorWidth,r=16);		//Motor
	}
	translate([0,0,spg30_encoderWidth+spg30_motorWidth])
	difference()
	{
		color("dimgray")
		cylinder(h=spg30_gearWidth,r=18.5); 	//Getriebe

		translate([15.5,0,18.5])
		cylinder(h=3.55,r=1.5);	//Schraublöcher
		translate([-15.5,0,18.5])
		cylinder(h=3.55,r=1.5);
	}
	color("dimgray")
	{
		translate([0,7,	spg30_gearWidth+
							spg30_motorWidth+
							spg30_encoderWidth])
		cylinder(h=6,r=6);			//Kugellager
		translate([0,7,	spg30_gearWidth+
							spg30_motorWidth+
							spg30_encoderWidth+
							6])
		cylinder(h=15.5,r=3); 	//Welle Rad
	}
}

/////Motorhalter//////

module motorholder()
{
	translate([0,0,7])
	{
		color("darkgray")
		{
			difference()
			{
				rotate(0) //Als Abschnitt zusammenfassen
				{
					intersection()
					{
						translate([0,9.3975,60.79])
						cube([37.59,18.795,1.59],true);
						translate([0,0,60])
						cylinder(h=1.59,r=18.795);
					}
					translate([0,-9.3975,60.79])
					cube([37.59,18.795,1.59],true);
				}	
	
				translate([15.5,0,59.99]) //Schraublöcher
				cylinder(h=1.61,r=1.5);
				translate([-15.5,0,59.99])
				cylinder(h=1.61,r=1.5);

				translate([0,7.01,59.99]) //Loch für Kugellager
				cylinder(h=1.6,r=6.605);
				translate([0,-7.01,59.99])
				cylinder(h=1.6,r=6.605);
				translate([0,0,60.79])
				cube([13.21,14.02,1.6],true);
			}
			
			translate([0,-19.59,35.39])
			{
				difference()
				{
					cube([37.59,1.59,49.21],true);
					translate([-15.62,0,0])
					for(j = [0:1])
					{
						for ( i = [0:6] )
						{
							translate([31.24*j,0.8,19.05-(i*6.35)])
							{
								rotate([90,0,0])
								{
									translate([0,0,-0.2])
									cylinder(h=2,r=1.59);
								}
							}
						}
					}
				}
			}
		}
	}
}


module wheel(diameter,width)
{
	translate([0,7,77])
	color("grey")
	cylinder(h=width,r=diameter/2);
}

////Befestigung/////////////////////IR///////////////////////////////////
module ir_mounting(height)
{
	difference()
	{
		translate([0,0,-height/2])
		cube([3,46,height],true);
	
		rotate([0,90,0])
		{
			translate([height-4.5,0,0])
			{
				translate([-1,(37/2),-2])
				cylinder(h=5,r=drill_m3/2);
				translate([-1,-(37/2),-2])
				cylinder(h=5,r=drill_m3/2);
			}
		}
	}
}

module ir_mounting_hole()
{
	cube([3.15,46.15,20],true);
}

//SRF10
module srf10_mounting(height)
{
	difference()
	{
		translate([-17,-4,0])
		cube([34,height,3]);
		translate([-9,4,-4])
		cylinder(h=8,r=4.9);
		translate([9,4,-4])
		cylinder(h=8,r=4.9);
	}
}

module srf10_mounting_hole()
{
	cube([34.15,10,3.15],true);
}
/////////////LED Heatsink//////////////
module LED_Heatsink()
{
	color("black")
	cube([15.3,15.3,8],true);
}

//////////////Rescuekit Abwerfer/////

module kitdropper_sidewall(length)
{
	%color("white")
	{
		difference()
		{
			translate([18,0,5.25])  //Große Seitenabdeckung
			cube([length+1+36,3,81.5],true);
		
			translate([27.5,0,21]) //Aussparung große Fläche
			cube([37,4,70],true);
		
			translate([27,0,13]) //Aussparung Befestigung Servohalter
			cube([25,4,60],true);
		
			translate([length/2-1+0.5,0,6]) //Aussparung Rückwand
			cube([4,4,30],true);
		
			translate([-8,0,6]) //Aussparung vorderwand
			cube([4,4,30],true);
		}
	}
}

module kitdropper_frontbackwall(width, length)
{
	difference() //Vorderwand
	{
		translate([-(length/2-1),0,8.75]) 
		cube([3,width+6,74.5],true);
		
		translate([-9.5,13.5,-29])
		cube([4,5,20]);
		translate([-9.5,-18.5,-29])
		cube([4,5,20]);

		translate([-9.5,13.5,21])
		cube([4,5,30]);
		translate([-9.5,-18.5,21])
		cube([4,5,30]);
	}
}

module kitdropper(width, length, dropperpos)
{
	%color("white")
	{
		translate([0,-(width/2+1.5),0])
		kitdropper_sidewall(length);
		translate([0,-(width/2-1.5),8.75])  //Schacht schmaler
		cube([length-5.8,3,81.5-7],true);
		difference() //Befestigungsleiste
		{
			translate([18,length/2+12,-34])
			cube([length+1+36,8,3],true);
			
			translate([0,length/2+13,-37])
			cylinder(h = 5, r = 1.5);
			translate([36,length/2+13,-37])
			cylinder(h = 5, r = 1.5);
		}
		
		translate([0,width/2+1.5,0])
		kitdropper_sidewall(length);
		translate([0,width/2-1.5,8.75])  //Schacht schmaler
		cube([length-5.8,3,81.5-7],true);
		difference()
		{
			translate([18,-(length/2+12),-34])
			cube([length+1+36,8,3],true);
			
			translate([0,-(length/2+13),-37])
			cylinder(h = 5, r = 1.5);
			translate([36,-(length/2+13),-37])
			cylinder(h = 5, r = 1.5);
		}
		
		kitdropper_frontbackwall(width, length);
		translate([length-2,0,0])
		kitdropper_frontbackwall(width, length);
	}
	color("white")
	{
		translate([dropperpos+5,0,-32.95])
		difference()
		{
			translate([-15.5/2+1.5,0,0])
			cube([length+15+15.5,width-1,5],true);
			
			hull()
			{
				translate([length/2+2.5,-(width/2-3.5),-5])
				cylinder(h=10,r=2);
				translate([length/2+2.5,width/2-3.5,-5])
				cylinder(h=10,r=2);
			}
			translate([-21,0,0])
			cube([13,22,10],true);
		}
	}
	
	%color("white")
	difference()
	{
		translate([27,0,-15.5])
		cube([36,33,3],true);
	
		hull() //Servobefestigung 1
		{
			translate([13,10,-18])
			cylinder(h=5,r=1.5);
			translate([13,-10,-18])
			cylinder(h=5,r=1.5);
		}
		
		translate([27,0,-16.5]) //Aussparung Servo
		minkowski()
		{
			cube([18,22,2],true);
			cylinder(h=2,r=2);
		}
		
		hull() //Servobefestigung 2
		{
			translate([41,10,-18])
			cylinder(h=5,r=1.5);
			translate([41,-10,-18])
			cylinder(h=5,r=1.5);
		}
		
		translate([7.5,13.5,-17.5]) //Aussparungen an den vier Ecken
		cube([7,4,4]);
	
		translate([7.5,-17.5,-17.5])
		cube([7,4,4]);
	
		translate([39.5,-17.5,-17.5])
		cube([7,4,4]);
	
		translate([39.5,13.5,-17.5])
		cube([7,4,4]);
	}
	translate([10,7,-0.5])
	rotate([0,180,0])
	servo_micro();
	
	translate([0,0,-32.45])
	for(i = [0:11])
	{
		color("red")
		translate([0,0,i*6.2])
		cube([length-6,width-7,6],true);
	}

}

module rescuekit()
{
	color("dimgray")
	cylinder(h=5.5,r=5.25);
	translate([0,0,-0.2])
	color("silver")
	cylinder(h=6.1,r=3.5);

	difference()
	{
		translate([-4.5,-10,-0.25])
		cube([9,15,6]);
		translate([0,0,-2])
		cylinder(h=10,r=4.8);
	}
}
