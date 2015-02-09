$fn=20;

include <electro.scad>
include <mechanics.scad>
///////////////////////////////////////////////////////////////////
///////////////////////////Hauptmodell/////////////////////////////
///////////////////////////////////////////////////////////////////

//Top view:

achsenabstand = 120;
motorabstand = 1; //Luft zwischen zwei Motoren (an Encodern + Kabel)
wheel_diameter = 54;
wheel_width = 20;
drill_m3 = 2.5;
sensdist_margin_sm = 35;
sensdist_margin_me = 35;
sensdist_margin_bi = 65;

upperplate_thickness = 4;
lowerplate_thickness = 3;

ir_sensorholder_height_lr = 17; //Halterung für die linken/rechten IR Sensoren (Höhe)
ir_sensorholder_height_fb = 30; //" - vorne/hinten
srf_sensorholder_height = 22.5;

sens_heighth = 5;

sharp_ray_length = 0;
cam_fov_on = 0;
srf10_fov_on = 0;
mlx_fov_on = 0;//90/2;

mlx_rot_x = 0;
mlx_rot_y = 90;
mlx_rot_z = 0;

mlx_angle = 20;
cmucam_angle = 50;
srf10_upperplate_angle = 3;

module screw(angle)
{
	translate([0,0,44])
	cylinder(h=5,r=1.2);
	translate([0,0,48.5])
	{
		difference()
		{
			cylinder(h = 2, r = 2.5);
			translate([0,0,2])
			rotate([0,0,angle])
			cube([1000,0.7,2],true);
		}
	}
}

translate([0,0,58+78.06]) //Runde Boardplatte 2 (UI + Schutz)
{
	%color("white")
	{
		difference()
		{
			//roundCornersCube(190,190,3,20);
			translate([0,0,-0.75])
			minkowski()
			{
				//Breite: 150 + 2*20
				cube([150,150,upperplate_thickness/2],true);
				cylinder(r=20,h=1.5);
			}

			translate([-90,45,-4.5])	//Löcher
			cylinder(h=9,r=1.5);
			translate([90,45,-4.5])
			cylinder(h=9,r=1.5);
			translate([90,-45,-4.5])
			cylinder(h=9,r=1.5);
			translate([-90,-45,-4.5])
			cylinder(h=9,r=1.5);

			translate([-52.5,-44,-4.5]) //Löcher (LCD)
			cylinder(h=9,r=1.5);
			translate([12.5,-44,-4.5])
			cylinder(h=9,r=1.5);
			translate([-52.5,44,-4.5])
			cylinder(h=9,r=1.5);
			translate([12.5,44,-4.5])
			cylinder(h=9,r=1.5);

			translate([-70,-62,-4.5]) //Löcher Wärmesensoren
			cylinder(h=7,r=drill_m3/2);
			translate([70,-62,-4.5])
			cylinder(h=7,r=drill_m3/2);
			translate([70,62,-4.5])
			cylinder(h=7,r=drill_m3/2);
			translate([-70,62,-4.5])
			cylinder(h=7,r=drill_m3/2);

			translate([-45,85,-4.5]) //Löcher UIunit
			cylinder(h=9,r=1.5);
			translate([5,85,-4.5])
			cylinder(h=9,r=1.5);
			translate([-2,85,-4.5]) //LEDs
			cylinder(h=9,r=2.5);
			translate([-38,85,-4.5])
			cylinder(h=9,r=2.5);

			translate([-5,63,-2.5])
			cylinder(h=5,r=3); //Inkrementalgeber

			translate([-35,63,-2.5])
			cylinder(h=5,r=3); //Taster

			translate([-20,-63,0])
			cube([19.2,12.5,5],true); //Schalter (Power)

			translate([86,0,0]) //Aussparung Kitdropper
			cube([18.1,33.1,4],true);
		}
	}

	color("silver")
	{
		rotate([0,0,0])
		{
			translate([-90,45,-46.75])	//Verbinder
			{
				cylinder(h=45,r=2.5, $fn = 6);				
				screw(20);
			}
			translate([90,45,-46.75])
			{
				cylinder(h=45,r=2.5, $fn = 6);
				screw(40);
			}
			translate([90,-45,-46.75])
			{
				cylinder(h=45,r=2.5, $fn = 6);
				screw(170);
			}
			translate([-90,-45,-46.75])
			{
				cylinder(h=45,r=2.5, $fn = 6);
				screw(270);
			}
		}

		translate([-45,85,-11.5])		//Verbinder UIunit
		cylinder(h=10,r=2.5, $fn = 6);
		translate([5,85,-11.5])
		cylinder(h=10,r=2.5, $fn = 6);

		translate([-52.5,-44,-11.5]) //Verbinder LCD
		cylinder(h=10,r=2.5, $fn = 6);
		translate([12.5,-44,-11.5])
		cylinder(h=10,r=2.5, $fn = 6);
		translate([-52.5,44, -11.5])
		cylinder(h=10,r=2.5, $fn = 6);
		translate([12.5,44,-11.5])
		cylinder(h=10,r=2.5, $fn = 6);

		translate([-70,-62,-11.7]) //Verbinder Wärmesensoren
		rotate([0,0,180])
		cylinder(h=10,r=2.5, $fn = 6);

		translate([70,-62,-1.7])
		rotate([0,180,180])
		cylinder(h=10,r=2.5, $fn = 6);
	
		translate([70,62,-11.7])
		rotate([180,180,180])
		cylinder(h=10,r=2.5, $fn = 6);
	
		translate([-70,62,-1.7])
		rotate([180,0,180])
		cylinder(h=10,r=2.5, $fn = 6);

	}

	translate([-20,0,-12])
	lcd();

	translate([-20,-63,-3])
	switch();

	translate([-35,63,-10])
	pushbutton();

	translate([-20,85,-12])
	uiunit(10);

	translate([-5,63,-4])
	{
		incremental();
		incremental_btn();
	}

	translate([-70,-70,-13.75]) //IR Sensoren
	rotate([-mlx_angle,0,180])
	mlx(mlx_rot_x,mlx_rot_y,mlx_rot_z,mlx_fov_on);

	translate([70,-70,-13.75]) //IR Sensoren
	rotate([mlx_angle,180,180])
	mlx(mlx_rot_x,mlx_rot_y,mlx_rot_z,mlx_fov_on);
	
	translate([70,70,-13.75]) //IR Sensoren
	rotate([180-mlx_angle,180,180])
	mlx(mlx_rot_x,mlx_rot_y,mlx_rot_z,mlx_fov_on);
	
	translate([-70,70,-13.75]) //IR Sensoren
	rotate([180+mlx_angle,0,180])
	mlx(mlx_rot_x,mlx_rot_y,mlx_rot_z,mlx_fov_on);
}

translate([0,0,58+29.81]) //Runde Boardplatte (Sensoren, Mainboard etc.)
{
	%color("white")
	{
		difference()
		{
			translate([0,0,-0.75])
			minkowski()
			{
				cube([150,150,1.5],true);
				cylinder(r=20,h=1.5);
			}
			translate([-100,0,-3])
			cylinder(r=30,h=5);

			translate([0,20,-2])
			minkowski()
			{
				cube([30,72,1.5],true);
				cylinder(r=16,h=3);
			}

			translate([-90,45,-4.5])	//Löcher
			cylinder(h=9,r=1.5);
			translate([90,45,-4.5])
			cylinder(h=9,r=1.5);
			translate([90,-45,-4.5])
			cylinder(h=9,r=1.5);
			translate([-90,-45,-4.5])
			cylinder(h=9,r=1.5);

			translate([20,0,0])
			{
				rotate([0,0,270])
				{
					translate([46,0,-3.5])    //Löcher PCB
					cylinder(h=7,r=drill_m3/2);
					translate([46,-36,-3.5])
					cylinder(h=7,r=drill_m3/2);
					translate([-42,36,-3.5])
					cylinder(h=7,r=drill_m3/2);
				}
			}
			
			translate([-45,23.749,-3.5])    //Löcher CMUcam
			cylinder(h=7,r=drill_m3/2);	
			translate([-45,-23.749,-3.5])
			cylinder(h=7,r=drill_m3/2);			

			translate([75,0,-1]) //Aussparung Kitdropper
			minkowski()
			{
				cube([60,23,2],true);
				cylinder(h=2,r=5);
			}

			translate([-sensdist_margin_me,0,sens_heighth])
			{
				ir_mounting_hole();
			}
			*translate([sensdist_margin_me,0,sens_heighth])
			{
				ir_mounting_hole();
			}
		
			translate([-sensdist_margin_sm,-sensdist_margin_bi,sens_heighth])
			{
				ir_mounting_hole();
			}
			translate([-sensdist_margin_sm,sensdist_margin_bi,sens_heighth])
			{
				ir_mounting_hole();
			}
			translate([sensdist_margin_sm,-sensdist_margin_bi,sens_heighth])
			{
				ir_mounting_hole();
			}
			translate([sensdist_margin_sm,sensdist_margin_bi,sens_heighth])
			{
				ir_mounting_hole();
			}
			translate([sensdist_margin_bi,sensdist_margin_sm,sens_heighth])
			{
				rotate([0,0,90])
				ir_mounting_hole();
			}
			translate([sensdist_margin_bi,-sensdist_margin_sm,sens_heighth])
			{
				rotate([0,0,90])
				ir_mounting_hole();
			}
			translate([-sensdist_margin_bi,sensdist_margin_sm,sens_heighth])
			{
				rotate([0,0,90])
				ir_mounting_hole();
			}
			translate([-sensdist_margin_bi,-sensdist_margin_sm,sens_heighth])
			{
				rotate([0,0,90])
				ir_mounting_hole();
			}

			rotate([90,0,90-srf10_upperplate_angle])
			translate([60,0,-75])
			srf10_mounting_hole();

			rotate([90,0,90+srf10_upperplate_angle]) 
			translate([-60,0,-75])
			srf10_mounting_hole();
		}
		translate([-sensdist_margin_me,0,lowerplate_thickness/2])
		{
			ir_mounting(ir_sensorholder_height_fb);
		}
		*translate([sensdist_margin_me,0,lowerplate_thickness/2])
		{
			ir_mounting(ir_sensorholder_height_fb);
		}
		
		translate([-sensdist_margin_sm,-sensdist_margin_bi,lowerplate_thickness/2])
		{
			ir_mounting(ir_sensorholder_height_fb);
		}
		translate([-sensdist_margin_sm,sensdist_margin_bi,lowerplate_thickness/2])
		{
			ir_mounting(ir_sensorholder_height_fb);
		}
		translate([sensdist_margin_sm,-sensdist_margin_bi,lowerplate_thickness/2])
		{
			ir_mounting(ir_sensorholder_height_fb);
		}
		translate([sensdist_margin_sm,sensdist_margin_bi,lowerplate_thickness/2])
		{
			ir_mounting(ir_sensorholder_height_fb);
		}
		translate([sensdist_margin_bi,sensdist_margin_sm,lowerplate_thickness/2])
		{
			rotate([0,0,90])
			ir_mounting(ir_sensorholder_height_lr);
		}
		translate([sensdist_margin_bi,-sensdist_margin_sm,lowerplate_thickness/2])
		{
			rotate([0,0,90])
			ir_mounting(ir_sensorholder_height_lr);
		}
		translate([-sensdist_margin_bi,sensdist_margin_sm,lowerplate_thickness/2])
		{
			rotate([0,0,90])
			ir_mounting(ir_sensorholder_height_lr);
		}
		translate([-sensdist_margin_bi,-sensdist_margin_sm,lowerplate_thickness/2])
		{
			rotate([0,0,90])
			ir_mounting(ir_sensorholder_height_lr);
		}

		rotate([270,0,90-srf10_upperplate_angle]) //Zwei SRF10 Halter
		translate([60,-17,73.5])
		srf10_mounting(srf_sensorholder_height_lr);

		rotate([270,0,90+srf10_upperplate_angle]) 
		translate([-60,-17,73.5])
		srf10_mounting(srf_sensorholder_height_lr);
	}

	translate([-sensdist_margin_me-1,0,-ir_sensorholder_height_fb-lowerplate_thickness+10]) //IR mitte vorne
	{
		rotate([270,180,90])
		{
			sharp_IR_new(sharp_ray_length);
		}
	}
	*translate([sensdist_margin_me+1,0,-ir_sensorholder_height_fb-lowerplate_thickness+10]) //IR mitte hinten
	{
		rotate([90,0,90])
		{
			sharp_IR_short(sharp_ray_length, 1);
		}
	}

	translate([sensdist_margin_sm+1,sensdist_margin_bi,-ir_sensorholder_height_fb-lowerplate_thickness+10]) //IR Sensor hinten rechts
	{
		rotate([90,0,90])
		{
			sharp_IR_short(sharp_ray_length, 1);
		}
	}

	translate([sensdist_margin_sm+1,-sensdist_margin_bi,-ir_sensorholder_height_fb-lowerplate_thickness+10]) //IR Sensor hinten links
	{
		rotate([90,0,90])
		{
			sharp_IR_short(sharp_ray_length, 1);
		}
	}
	translate([-sensdist_margin_sm-1,sensdist_margin_bi,-ir_sensorholder_height_fb-lowerplate_thickness+10]) //IR Sensor vorne rechts
	{
		rotate([270,180,90])
		{
			sharp_IR_short(sharp_ray_length, 1);
		}
	}
	translate([-sensdist_margin_sm-1,-sensdist_margin_bi,-ir_sensorholder_height_fb-lowerplate_thickness+10]) //IR Sensoren
	{
		rotate([270,180,90])
		{
			sharp_IR_short(sharp_ray_length, 1);
		}
	}

	translate([sensdist_margin_bi,sensdist_margin_sm+1,-ir_sensorholder_height_lr-lowerplate_thickness+10]) //IR Sensor rechts hinten
	{
		rotate([270,0,0])
		{
			sharp_IR_short(sharp_ray_length, 1);
		}
	}
	translate([-sensdist_margin_bi,sensdist_margin_sm+1,-ir_sensorholder_height_lr-lowerplate_thickness+10]) //IR Sensor rechts vorne
	{
		rotate([270,0,0])
		{
			sharp_IR_short(sharp_ray_length, 1);
		}
	}
	translate([-sensdist_margin_bi,-sensdist_margin_sm-1,-ir_sensorholder_height_lr-lowerplate_thickness+10]) //IR Sensor links vorne
	{
		rotate([270,0,180])
		{
			sharp_IR_short(sharp_ray_length, 1);
		}
	}
	translate([sensdist_margin_bi,-sensdist_margin_sm-1,-ir_sensorholder_height_lr-lowerplate_thickness+10]) //IR Sensor links hinten
	{
		rotate([270,0,180])
		{
			sharp_IR_short(sharp_ray_length, 1);
		}
	}

	color("silver")
	{
		translate([-90,45,-31.5])	//Verbinder
		cylinder(h=30,r=2.5, $fn = 6);
		translate([90,45,-31.5])
		cylinder(h=30,r=2.5, $fn = 6);
		translate([90,-45,-31.5])
		cylinder(h=30,r=2.5, $fn = 6);
		translate([-90,-45,-31.5])
		cylinder(h=30,r=2.5, $fn = 6);

		translate([20,0,-1.5])
		{
			rotate([0,0,270])
			{
				translate([46,0,3])   	//Verbinder Platine
				cylinder(h=15,r=2.5, $fn = 6);
				translate([46,-36,3])
				cylinder(h=15,r=2.5, $fn = 6);
				translate([-42,-36,3])
				cylinder(h=15,r=2.5, $fn = 6);
				translate([-42,36,3])
				cylinder(h=15,r=2.5, $fn = 6);
			}
		}
	}
	
	translate([0,0,0])
	{
		rotate([90,0,270+srf10_upperplate_angle]) //Zwei SRF10
		translate([60,13,70])
		srf10(srf10_fov_on);

		rotate([90,180,270-srf10_upperplate_angle]) 
		translate([60,-13,70])
		srf10(srf10_fov_on);

		translate([-60,0,25]) //CMUcam
		rotate([0,cmucam_angle,0])	
		cmucam(cam_fov_on);

		translate([20,0,2.3+15]) //Mainboard
		rotate([0,0,270])		
		pcb_main(1,1);

		translate([-23,0,2.2])
		adcexpansion();
	}
}

translate([0,0,53.21]) //Chassis Hauptplatte
{
	%color("white")
	{
		difference()
		{
			translate([0,0,-1.4])
			minkowski()
			{
				//Breite: 150 + 2*20
				cube([180,	motorabstand+
						spg30_gearWidth*2+
						spg30_motorWidth*2+
						spg30_encoderWidth*2
						-10
						,3]	
						,true);
				cylinder(r=5,h=3);
			}

			rotate([90,0,0])
			{
				translate([44.39,4,37.9])
				{
					for(j = [0:1])
					{
						for ( i = [0:2] )
						{
							translate([31.24*j,0.8,19.05-(i*6.35*3)])
							{
								rotate([90,0,0])
								{
									cylinder(h=9,r=drill_m3/2);
								}
							}
						}
					}
				}
				translate([44.39,4,-37.9])
				{
					for(j = [0:1])
					{
						for ( i = [0:2] )
						{
							translate([31.24*j,0.8,19.05-(i*6.35*3)])
							{
								rotate([90,0,0])
								{
									cylinder(h=9,r=drill_m3/2);
								}
							}
						}
					}
				}
				translate([-44.39-31.24,4,37.9])
				{
					for(j = [0:1])
					{
						for ( i = [0:2] )
						{
							translate([31.24*j,0.8,19.05-(i*6.35*3)])
							{
								rotate([90,0,0])
								{
									cylinder(h=9,r=drill_m3/2);
								}
							}
						}
					}
				}
				translate([-44.39-31.24,4,-37.9])
				{
					for(j = [0:1])
					{
						for ( i = [0:2] )
						{
							translate([31.24*j,0.8,19.05-(i*6.35*3)])
							{
								rotate([90,0,0])
								{
									cylinder(h=9,r=drill_m3/2);
								}
							}
						}
					}
				}
			}

			translate([-90,45,-4.5])	//Löcher für 2. Etage
			cylinder(h=9,r=drill_m3/2);
			translate([90,45,-4.5])
			cylinder(h=9,r=drill_m3/2);
			translate([90,-45,-4.5])
			cylinder(h=9,r=drill_m3/2);
			translate([-90,-45,-4.5])
			cylinder(h=9,r=drill_m3/2);
			
			translate([-72.5,0,0])
			{
				translate([55,32.5,-3.5])    //Löcher Power Unit
				cylinder(h=7,r=drill_m3/2);
				translate([90,32.5,-3.5])
				cylinder(h=7,r=drill_m3/2);
				translate([55,-32.5,-3.5])
				cylinder(h=7,r=drill_m3/2);
				translate([90,-32.5,-3.5])
				cylinder(h=7,r=drill_m3/2);
			}			

			translate([4.5,46,-4.5])	//UM6 Loch
			cylinder(h=9,r=drill_m3/2);

			translate([0,-52,-4.5])	//LED Treiber Loch
			cylinder(h=9,r=drill_m3/2);

			translate([-65,0,-4.5])	//Loch Groundsens
			cylinder(h=9,r=drill_m3/2);

			translate([86,21.5,-4.5])	//Löcher Kitdropper
			cylinder(h=9,r=drill_m3/2);
			translate([86,-21.5,-4.5])
			cylinder(h=9,r=drill_m3/2);
			translate([50,21.5,-4.5])
			cylinder(h=9,r=drill_m3/2);
			translate([50,-21.5,-4.5])
			cylinder(h=9,r=drill_m3/2);

			translate([88,16.5,-4.5])	//Löcher IRdist unten
			cylinder(h=9,r=drill_m3/2);
			translate([88,53.5,-4.5])
			cylinder(h=9,r=drill_m3/2);

			translate([-92,-25,-4.5]) //Löcher Untergrundsensor Option 1
			cylinder(h=9,r=drill_m3/2);
			translate([-92,-55,-4.5])
			cylinder(h=9,r=drill_m3/2);		
			translate([-82,-25,-4.5])
			cylinder(h=9,r=drill_m3/2);
			translate([-82,-55,-4.5])
			cylinder(h=9,r=drill_m3/2);		

			translate([-92,25,-4.5])  //Löcher Untergrundsensor Option 2
			cylinder(h=9,r=drill_m3/2);
			translate([-92,55,-4.5])
			cylinder(h=9,r=drill_m3/2);		
			translate([-82,25,-4.5])
			cylinder(h=9,r=drill_m3/2);
			translate([-82,55,-4.5])
			cylinder(h=9,r=drill_m3/2);		

			hull() //Kabeldurchgänge
			{
				translate([-30,0,-4.5]) 
				cylinder(h=9,r=10);
				translate([-30,-25,-4.5])
				cylinder(h=9,r=10);
			}
			hull()
			{
				translate([30,0,-4.5])
				cylinder(h=9,r=10);
				translate([30,-25,-4.5])
				cylinder(h=9,r=10);
			}

			translate([-85,0,0]) //SRF10 Loch
			rotate([270,0,90])
			srf10_mounting_hole();

			translate([81,0,3]) //IR Dist Loch
			ir_mounting_hole();
		}

		translate([-86.5,0,-16]) //SRF10 Halter
		rotate([90,0,90])
		srf10_mounting(srf_sensorholder_height);

		translate([81,0,3]) //IR Dist Halter
		ir_mounting(ir_sensorholder_height_lr);
	}

	translate([-80,0,-12]) //SRF10
	rotate([270,0,90])
	srf10(srf10_fov_on);

	translate([82,0,-10]) //IR Dist
	rotate([90,0,90])
	sharp_IR_new(0);

	translate([0,0,13]) //Powerunit
	rotate([0,0,180])
	powerunit();
	
	translate([0,50,13]) //UM6
	um6(1);

	translate([86,0,38.6]) //Kitdropper
	rotate([0,0,180])
	kitdropper(27,17,16);

	color("silver")
	{
		translate([-72.5,0,0])
		{
			translate([55,32.5,3])    //Verbinder Power Unit
			cylinder(h=10,r=2.5, $fn = 6);
			translate([90,32.5,3])
			cylinder(h=10,r=2.5, $fn = 6);
			translate([55,-32.5,3])
			cylinder(h=10,r=2.5, $fn = 6);
			translate([90,-32.5,3])
			cylinder(h=10,r=2.5, $fn = 6);
		}

		translate([-92,-25,-13])  //Verbinder Untergrundsensor Option 2
		cylinder(h=10,r=2.5, $fn = 6);
		translate([-92,-55,-13])
		cylinder(h=10,r=2.5, $fn = 6);
		translate([-82,-25,-23])
		cylinder(h=20,r=2.5, $fn = 6);
		translate([-82,-55,-13])
		cylinder(h=10,r=2.5, $fn = 6);
		
		translate([4.5,46,3])	//UM6 Verbinder
		cylinder(h=10,r=2.5, $fn = 6);

		translate([0,-52,3])	//LED Treiber Verbinder
		cylinder(h=10,r=2.5, $fn = 6);
	}

	translate([88,35,-3])
	rotate([180,0,90])
	sharp_IR_short(sharp_ray_length, 0);

	translate([-87,-40,-13.6]) //HP-LED für groundsens
	{
		color("chocolate")
		cube([16,35,1.6],true);
		rotate([180,0,0])
		translate([0,0,0.7])
		HP_LED();
		color("black")
		translate([0,0,4])
		cube([12,12,7],true);
	}

	color("dimgray") //LED Treiber
	translate([0,-52,20])
	cube([45,25,15],true);
}


translate([0,0,39])		//Akku
{
	color("black")
	cube([31,100,23],true);
}

/////////Untergrundsensor////////////


////////Motoren, Halter und Räder////////

translate([achsenabstand/2,motorabstand/2-5,30])		//oben rechts
{
	rotate([270,0,0])
	{
		motorholder();
		spg30();
		wheel(wheel_diameter,wheel_width);
	}
}

translate([-achsenabstand/2,motorabstand/2-5,30])		//oben rechts
{
	rotate([270,0,0])
	{
		motorholder();
		spg30();
		wheel(wheel_diameter,wheel_width);
	}
}

translate([achsenabstand/2,-motorabstand/2+5,30])		//oben rechts
{
	rotate([90,180,0])
	{
		motorholder();
		spg30();
		wheel(wheel_diameter,wheel_width);
	}
}

translate([-achsenabstand/2,-motorabstand/2+5,30])		//oben rechts
{
	rotate([90,180,0])
	{
		motorholder();
		spg30();
		wheel(wheel_diameter,wheel_width);
	}
}

//////////////////UMGEBUNG///////////
*translate([0,0,0])
{
	color("white") //Platte
	{
		translate([0,0,-9])
		cube([300,300,10],true);
		translate([-150,155,66])
		cube([600,10,160],true);
	}
	color("black")
	{
		translate([-300,0,-9])
		cube([300,300,10],true);
	}
	color("silver") //Opfer
	{
		translate([0,147,70])
		rotate([0,90,0])
		cube([75,7,35],true);
	}
}