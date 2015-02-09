$fn = 100;

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

projection(cut = true)
srf10_mounting(23);