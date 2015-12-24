////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2015//////////////////////////////////
/////////////////////////////////drive.c////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///    All driving functions
///            - 90° left
///            - 90° right
///            - 30cm straight forward
///            - stuff (reverse, ramp etc.
///            -> everything that has nothing to do with the mapping and navigation!
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "drive.h"
#include "maze.h"
#include "mazefunctions.h"
#include "um6.h"
#include "funktionen.h"
#include "system.h"
#include "bluetooth.h"
#include "i2cdev.h"
#include "irdist.h"

/*!
 * \
 *
 */
void drive_limitSpeed(int16_t *speed_l, int16_t *speed_r, int8_t limit)
{
    if(*speed_l > limit)
        *speed_l = limit;
    if(*speed_l < -limit)
        *speed_l  = -limit;

    if(*speed_r > limit)
        *speed_r = limit;
    if(*speed_r < -limit)
        *speed_r  = -limit;
}


/**
 * \brief drive_oneTile
 *
 *        Drive one tile straight forward
 *
 * \param data structure to drive one tile configuration
 */
void drive_oneTile(DOT *d)
{
    if(d->maxspeed == 0) //Set to constant if not initialized
        d->maxspeed = MAXSPEED;

    int16_t speed_tmp = 0;
    int16_t th_align_front = TILE1_FRONT_TH_FRONT;

    int16_t rel_angle = 0;

    int16_t robot_angleToRightWall = robot_getAngleToWall(EAST);
    int16_t robot_angleToLeftWall = robot_getAngleToWall(WEST);

    switch(d->state)
    {
        case DOT_INIT:
                d->abort = 0; //Should never be 1 on this point -> Reset it in case it is still set

                d->enc_lr_add = 0;

                d->um6_phi_t_start = um6.phi_t;

                d->aligned_turn = NONE;

                if(dist[FRONT][FRONT] < TILE1_FRONT_FRONT) //There is a wall directly in front of the robot
                {
                    d->state = DOT_END; //Break because we can`t drive straight
                    foutf(&str_debugDrive, "%i: dot:noSt_dst\n\r", timer);
                }
                else
                {
                    if((dist[BACK][BACK] < TILE1_BACK_TH_BACK) && //Enough place in back of the robot
                        (maze_getWall(&robot.pos, robot.dir+2) > MAZE_ISWALL)) //AND there is a wall in the map (prevent false sensordata...)
                    {
                        d->state = DOT_ALIGN_BACK;
                    }
                    else //Directly jump to the drive part
                    {
                        d->state = DOT_DRIVE;
                        d->enc_comp[LEFT] = mot.d[LEFT].enc;
                        d->enc_comp[RIGHT] = mot.d[RIGHT].enc;
                        d->enc_lr_start = mot.enc;
                        d->timer = 0; //Unactivate timer
                    }

                    foutf(&str_debugDrive, "%i: dot:st.en: %i\n\r", timer, d->enc_lr_start);
                }
            break;

        case DOT_ALIGN_BACK:

            if(!drive_align_back(TILE1_BACK_BACK))
            {
                d->state = DOT_ALIGN;
                d->timer = 0; //Unactivate timer
            }
            break;

        case DOT_ALIGN:

            if(!drive_align())
            {
                d->state = DOT_DRIVE;
                d->enc_comp[LEFT] = mot.d[LEFT].enc;
                d->enc_comp[RIGHT] = mot.d[RIGHT].enc;
                d->timer = timer;
                d->enc_lr_start = mot.enc;
            }

            break;

        case DOT_DRIVE:

            ////////////Limit maximum speed (Ramp, speed bumps...)/////////////

            rel_angle = abs((d->um6_phi_t_start - um6.phi_t) * 2);
            if(rel_angle > 70)
                rel_angle = 70;

            speed_tmp = d->maxspeed - rel_angle;

            if(d->enc_lr_add > 0)
                speed_tmp /= 2;


            /////////regulation of distance left/right////////

            if(abs(robot_angleToLeftWall) < abs(robot_angleToRightWall))
            {
                d->steer = 0 - (dist[LEFT][BACK] - dist[LEFT][FRONT]) * KP_DOT_DIR;

                d->steer += (DIST_SOLL - dist[LEFT][FRONT]) * -KP_DOT_DIST;
            }
            else if(abs(robot_angleToLeftWall) > abs(robot_angleToRightWall))
            {
                d->steer = 0 - (dist[RIGHT][FRONT] - dist[RIGHT][BACK]) * KP_DOT_DIR;

                d->steer += (DIST_SOLL - dist[RIGHT][FRONT]) * KP_DOT_DIST;
            }
            else
            {
                d->steer = 0;
            }

            ////////////////////////////////////////////////////////////////////////
            ////////Goal reached? Collision? calculate speed

            if(d->abort)
            {
                if(mot.enc < d->enc_lr_start) //There is a request to drive back - drive back until we reach the same encoder dist as start
                {
                    d->state = DOT_COMP_ENC; //End, compare angle of the robot
                    mot.d[LEFT].speed.to = 0;
                    mot.d[RIGHT].speed.to = 0;
                }
                else
                {
                    speed_tmp /= 2;
                    mot.d[LEFT].speed.to = -40;//(speed_tmp + d->steer);
                    mot.d[RIGHT].speed.to = -40;//(speed_tmp - d->steer);
                }
            }
            else
            {
                if(d->enc_lr_add == 0) //If the robot did not had to pass an obstacle or so look wide forward
                {
                    th_align_front = TILE1_FRONT_TH_FRONT;
                }
                else //otherwise not
                {
                    th_align_front = 120;//TILE1_FRONT_TH_FRONT/2;
                }

                if(((dist[FRONT][RIGHT] < COLLISIONAVOIDANCE_SENS_TH_1) &&
                    (dist[FRONT][LEFT] >= COLLISIONAVOIDANCE_SENS_TH_2) &&
                    (dist[FRONT][FRONT] >= COLLISIONAVOIDANCE_SENS_TH_2) &&
                    (mot.enc < (d->enc_lr_start + (TILE_DIST_COLLISION_AV * ENC_FAC_CM_LR) + d->enc_lr_add/2)) &&
                    (rel_angle < 20)) ||

                    (get_bumpR() &&
                     (mot.enc < (d->enc_lr_start + (TILE_DIST_COLLISION_AV * ENC_FAC_CM_LR) + d->enc_lr_add/2)) &&
                     (rel_angle < 20)) ||
                   ((robot_angleToRightWall > 20) && (robot_angleToRightWall != GETANGLE_NOANGLE) &&
                    (dist[RIGHT][FRONT] < 15)))
                {
                    d->aligned_turn = WEST;

                    mot.d[LEFT].speed.to = -SPEED_COLLISION_AVOIDANCE;
                    mot.d[RIGHT].speed.to = SPEED_COLLISION_AVOIDANCE;
                }
                else if(((dist[FRONT][LEFT] < COLLISIONAVOIDANCE_SENS_TH_1) &&
                         (dist[FRONT][RIGHT] >= COLLISIONAVOIDANCE_SENS_TH_2) &&
                         (dist[FRONT][FRONT] >= COLLISIONAVOIDANCE_SENS_TH_2) &&
                         (mot.enc < (d->enc_lr_start + (TILE_DIST_COLLISION_AV * ENC_FAC_CM_LR) + d->enc_lr_add/2)) &&
                         (rel_angle < 20)) ||

                         (get_bumpL() &&
                         (mot.enc < (d->enc_lr_start + (TILE_DIST_COLLISION_AV * ENC_FAC_CM_LR) + d->enc_lr_add/2)) &&
                         (rel_angle < 20)) ||
                        ((robot_angleToLeftWall > 20) && (robot_angleToLeftWall != GETANGLE_NOANGLE) &&
                         (dist[LEFT][FRONT] < 15)))
                {
                    d->aligned_turn = EAST;

                    mot.d[LEFT].speed.to = SPEED_COLLISION_AVOIDANCE;
                    mot.d[RIGHT].speed.to = -SPEED_COLLISION_AVOIDANCE;
                }
                else if(d->aligned_turn == WEST) //Robot had to align via front sensors or bumpers. Now add a small alignment (turn a bit more in the corresponding direction)
                {
                    d->state = DOT_ROT_WEST;
                }
                else if(d->aligned_turn == EAST)
                {
                    d->state = DOT_ROT_EAST;
                }
                else if(((dist[FRONT][LEFT] < th_align_front) &&
                         (dist[FRONT][FRONT] < th_align_front)) ||
                        ((dist[FRONT][RIGHT] < th_align_front) &&
                         (dist[FRONT][FRONT] < th_align_front)) ||
                        ((dist[FRONT][LEFT] < th_align_front) &&
                         (dist[FRONT][RIGHT] < th_align_front)))
                {
                    if(d->timer == 0)
                    {
                        d->timer = timer;
                    }
                    else if((timer - d->timer) > TIMER_DOT_ALIGN)
                    {
                        d->state = DOT_COMP_ENC; //End, compare angle of the robot
                    }
                    else
                    {
                        int16_t frontval = dist[FRONT][FRONT];
                        int16_t sensdiff_fl = abs(dist[FRONT][FRONT] - dist[FRONT][LEFT]);
                        int16_t sensdiff_fr = abs(dist[FRONT][FRONT] - dist[FRONT][RIGHT]);
                        int16_t sensdiff_lr = abs(dist[FRONT][LEFT] - dist[FRONT][RIGHT]);

                        if((abs(sensdiff_fl) > abs(sensdiff_lr)) && (abs(sensdiff_fr) > abs(sensdiff_lr)))
                        {
                            frontval = dist[FRONT][LEFT];
                        }

                        d->steer = ((TILE1_FRONT_FRONT - frontval) * (-KP_ALIGN_FRONT));

                        mot.d[LEFT].speed.to = d->steer;
                        mot.d[RIGHT].speed.to = d->steer;

                        drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, speed_tmp);

                        if(abs(d->steer) <= STEER_ALIGN_BACK_END)
                        {
                            d->state = DOT_COMP_ENC; //End, compare angle of the robot
                            mot.d[LEFT].speed.to = 0;
                            mot.d[RIGHT].speed.to = 0;
                        }
                    }
                }
                else if(mot.enc > (d->enc_lr_start + (TILE_LENGTH * ENC_FAC_CM_LR) + d->enc_lr_add))  //driven 30cm
                {
                    d->state = DOT_COMP_ENC; //End, compare angle of the robot
                    mot.d[LEFT].speed.to = 0;
                    mot.d[RIGHT].speed.to = 0;
                }
                else //Normal driving forward
                {
                    mot.d[LEFT].speed.to = (speed_tmp - d->steer);
                    mot.d[RIGHT].speed.to = (speed_tmp + d->steer);
                }
            }

            drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, speed_tmp);

            break;

        case DOT_ROT_WEST:
            d->r.angle = -TURN_ANGLE_COLLISION_AVOIDED;
            drive_rotate(&d->r);

            if(d->r.state == ROTATE_FINISHED)
            {
                d->r.state = ROTATE_INIT; //Allow rotate function with this object to start again...

                d->state = DRIVE_ROT_STRAIGHT;
                d->aligned_turn = NONE;

                if(d->enc_lr_add < (DIST_ADD_COLLISION_MAX * ENC_FAC_CM_LR))
                {
                    d->enc_lr_add += DIST_ADD_COLLISION * ENC_FAC_CM_LR;
                }
            }

            break;

        case DOT_ROT_EAST:
            d->r.angle = TURN_ANGLE_COLLISION_AVOIDED;
            drive_rotate(&d->r);

            if(d->r.state == ROTATE_FINISHED)
            {
                d->r.state = ROTATE_INIT; //Allow rotate function with this object to start again...

                d->state = DRIVE_ROT_STRAIGHT;
                d->aligned_turn = NONE;

                if(d->enc_lr_add < (DIST_ADD_COLLISION_MAX * ENC_FAC_CM_LR))
                {
                    d->enc_lr_add += DIST_ADD_COLLISION * ENC_FAC_CM_LR;
                }
            }

            break;

        case DRIVE_ROT_STRAIGHT:

            if(drive_dist(0, 40, 2) == 0) //Drive one cm with speed 40 straight
            {
                d->state = DOT_DRIVE;
            }
            break;

        case DOT_COMP_ENC:

            if(abs((mot.d[RIGHT].enc - d->enc_comp[RIGHT]) - (mot.d[LEFT].enc - d->enc_comp[LEFT])) > 100)
            {
                if(robot_getAngleToWall(NONE) == GETANGLE_NOANGLE)
                {
                    d->corr_angle = ((mot.d[RIGHT].enc - d->enc_comp[RIGHT]) - (mot.d[LEFT].enc - d->enc_comp[LEFT]))/13;

                    if(d->corr_angle > 40)
                        d->corr_angle = 40;
                    else if(d->corr_angle < -40)
                        d->corr_angle = -40;

                    d->state = DOT_CORR;
                }
                else
                {
                    d->state = DOT_ALIGN_WALL;
                }
            }
            else
            {
                d->state = DOT_END;
            }

            break;

        case DOT_CORR:
            d->r.angle = d->corr_angle;
            drive_rotate(&d->r);

            if(d->r.state == ROTATE_FINISHED)
            {
                d->r.state = ROTATE_INIT; //Allow rotate function with this object to start again...
                d->state = DOT_END;
            }

            break;

        case DOT_ALIGN_WALL:

            if(!drive_align())
            {
                d->state = DOT_END;
            }

            break;

        case DOT_END:

            d->state = DOT_FINISHED;

            foutf(&str_debugDrive, "%i: dot:end\n\r", timer);

            break;

        case DOT_FINISHED:

            break;

        default:
            foutf(&str_error, "%i: ERR:sw[drv.01]:DEF\n\r", timer);
            fatal_err = 1;
            d->state = DOT_INIT;
            break;
    }
}

/**
 * \brief drive_rotate
 *
 *        rotates n degree (given in rotate data struct) based on odometry!
 *
 * \param r pointer to rotate data struct
 */
void drive_rotate(D_ROTATE *r)
{
    switch(r->state)
    {
        case ROTATE_INIT:

            r->um6_psi_t_start = um6.psi_t;
            r->enc_l_start = mot.d[LEFT].enc;
            r->enc_r_start = mot.d[RIGHT].enc;

            r->use_enc = 0;
            r->timer = 0; //Reset timer

            r->progress = 0;

            r->state = ROTATE;

            if(r->speed_limit == 0)
                r->speed_limit = MAXSPEED; //If speed limit not set, set it to MAXSPEED

            if(r->angle == 0)
                r->state = ROTATE_END; //If angle not set, break execution

            foutf(&str_debugDrive, "%i: drRot:st: %i\n\r", timer, r->um6_psi_t_start);

            break;

        case ROTATE:

            if(r->angle > 0)
                r->steer = ((r->um6_psi_t_start + r->angle + UM6_ROTATE_OFFSET) - um6.psi_t) * KP_ROTATE;
            else
                r->steer = ((r->um6_psi_t_start + r->angle - UM6_ROTATE_OFFSET) - um6.psi_t) * KP_ROTATE;

            r->progress = abs((um6.psi_t - r->um6_psi_t_start)*100)/abs(r->angle);

            if(check_um6 != 0)
            {
                r->use_enc = 1;
            }

            if(r->use_enc)
            {
                if(r->angle > 0)
                {
                    r->progress = abs(((mot.d[LEFT].enc - r->enc_l_start)*100)/abs(r->angle * ENC_DEGROTFAC));

                    if((mot.d[LEFT].enc < (r->enc_l_start + (ENC_DEGROTFAC * r->angle))) ||
                         (mot.d[RIGHT].enc > (r->enc_r_start + (ENC_DEGROTFAC * r->angle))))
                    {
                        r->steer = STEER_ROTATE_ENC;
                    }
                    else
                    {
                        r->state = ROTATE_END;
                        r->steer = 0;
                    }
                }
                else
                {
                    r->progress = abs(((mot.d[LEFT].enc - r->enc_l_start)*100)/abs(r->angle * ENC_DEGROTFAC));

                    if((mot.d[LEFT].enc > (r->enc_l_start + (ENC_DEGROTFAC * r->angle))) ||
                         (mot.d[RIGHT].enc < (r->enc_r_start + (ENC_DEGROTFAC * r->angle))))
                    {
                        r->steer = -STEER_ROTATE_ENC;
                    }
                    else
                    {
                        r->state = ROTATE_END;
                        r->steer = 0;
                    }
                }
            }
            else if(r->steer == 0)
            {
                r->state = ROTATE_END;
            }

            if((abs(r->steer) > STEER_ROTATE_TH_TIMER) && (r->timer != 0)) //If we start a new turn with another turn object and the timer has already startet, we reset the timer if we stand on another point
            {
                r->timer = 0;
            }
            else if((abs(r->steer) < STEER_ROTATE_TH_TIMER) && (r->timer == 0))
            {
                r->timer = timer;
            }
            else if((((timer - r->timer) > TIMER_ROTATE) ||
                    (r->steer == 0)) && (r->timer != 0))
            {
                r->state = ROTATE_END;
            }

            mot.d[LEFT].speed.to = r->steer;
            mot.d[RIGHT].speed.to = -r->steer;

            drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, r->speed_limit);

            break;

        case ROTATE_END:

            mot.d[LEFT].speed.to = 0;
            mot.d[RIGHT].speed.to = 0;

            r->state = ROTATE_FINISHED;

            foutf(&str_debugDrive, "%i: drRot:end\n\r", timer);

        case ROTATE_FINISHED:

            break;


        default:
            foutf(&str_error, "%i: ERR:sw[drv.02]:DEF\n\r", timer);
            fatal_err = 1;
            r->state = ROTATE_INIT;
            break;
    }
}

/**
 * \brief drive_align
 *
 *        Align to the walls via distance sensors.
 *        This function is really old, we had no time to
 *        make it instancable.
 * \return 1 if aligning, 0 if done
 */
int16_t steer_turn = 0;
uint8_t sm_driveAlign = 0;
uint32_t timer_align;

uint8_t drive_align(void)
{
    uint8_t returnvar = 1;

    switch(sm_driveAlign)
    {
        case 0:
            steer_turn = 0;
            timer_align = timer;

            sm_driveAlign = 1;

            break;

        case 1:
            if((dist[RIGHT][FRONT] < TILE1_SIDE_ALIGN_TH) &&
                 (dist[RIGHT][BACK] < TILE1_SIDE_ALIGN_TH) &&
                 (abs(dist[RIGHT][FRONT] - dist[RIGHT][BACK]) < TURN_SENSDIFF_MAX))
            {
                if(dist[RIGHT][FRONT] > dist[RIGHT][BACK])
                {
                    steer_turn = ((dist[RIGHT][BACK] + ALIGN_OFFSET_RIGHT) - dist[RIGHT][FRONT]) * -KP_ALIGN;
                }
                else
                {
                    steer_turn = (dist[RIGHT][FRONT] - (dist[RIGHT][BACK] + ALIGN_OFFSET_RIGHT)) * KP_ALIGN;
                }
            }
            else if((dist[LEFT][FRONT] < TILE1_SIDE_ALIGN_TH) &&
                            (dist[LEFT][BACK] < TILE1_SIDE_ALIGN_TH) &&
                            (abs(dist[LEFT][FRONT] - dist[LEFT][BACK]) < TURN_SENSDIFF_MAX))
            {
                if(dist[LEFT][FRONT] > dist[LEFT][BACK])
                {
                    steer_turn = ((dist[LEFT][BACK] + ALIGN_OFFSET_LEFT) - dist[LEFT][FRONT]) * KP_ALIGN;
                }
                else
                {
                    steer_turn = (dist[LEFT][FRONT] - (dist[LEFT][BACK] + ALIGN_OFFSET_LEFT)) * -KP_ALIGN;
                }
            }
            else if((dist[FRONT][LEFT] < TILE1_SIDE_ALIGN_TH) &&
                            (dist[FRONT][RIGHT] < TILE1_SIDE_ALIGN_TH) &&
                            (abs(dist[FRONT][LEFT] - dist[FRONT][RIGHT]) < TURN_SENSDIFF_MAX))
            {
                if(dist[FRONT][LEFT] < dist[FRONT][RIGHT])
                {
                    steer_turn = ((dist[FRONT][RIGHT] + ALIGN_OFFSET_FRONT) - dist[FRONT][LEFT]) * -KP_ALIGN;
                }
                else
                {
                    steer_turn = (dist[FRONT][LEFT] - (dist[FRONT][RIGHT] + ALIGN_OFFSET_FRONT)) * KP_ALIGN;
                }
            }
            else if((dist[BACK][LEFT] < TILE1_SIDE_ALIGN_TH) &&
                            (dist[BACK][RIGHT] < TILE1_SIDE_ALIGN_TH) &&
                            (abs(dist[BACK][LEFT] - dist[BACK][RIGHT]) < TURN_SENSDIFF_MAX))
            {
                if(dist[BACK][LEFT] < dist[BACK][RIGHT])
                {
                    steer_turn = ((dist[BACK][RIGHT] + ALIGN_OFFSET_BACK) - dist[BACK][LEFT]) * KP_ALIGN;
                }
                else
                {
                    steer_turn = (dist[BACK][LEFT] - (dist[BACK][RIGHT] + ALIGN_OFFSET_BACK)) * -KP_ALIGN;
                }
            }
            else
            {
                sm_driveAlign = 2;
            }

            if((abs(steer_turn) <= STEER_ALIGN_DONE) || (timer - timer_align > TIMER_ALIGN))
                sm_driveAlign = 2;

            break;

        case 2:
            steer_turn = 0;
            sm_driveAlign = 0;
            returnvar = 0;
            break;
    }

    mot.d[LEFT].speed.to = steer_turn;
    mot.d[RIGHT].speed.to = -steer_turn;

    drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, MAXSPEED);

    return returnvar;
}

uint8_t sm_dab = 0;
int16_t steer_dab = 0;

uint32_t timer_alignBack = 0;

/**
 * \brief drive_align_back
 *
 *        Align given distance to the wall behind the robot
 *
 * \param dist_to distance to align
 * \return 1 if aligning, 0 if done
 */
uint8_t drive_align_back(uint8_t dist_to) //Distance to the back
{
    uint8_t returnvar = 1;

    int16_t dist_back; //any of the distance sensor in the back
    if(dist[BACK][BACK] != IRDIST_MAX)
        dist_back = dist[BACK][BACK];
    else if(dist[BACK][LEFT] != IRDIST_MAX)
        dist_back = dist[BACK][LEFT];
    else if(dist[BACK][RIGHT] != IRDIST_MAX)
        dist_back = dist[BACK][RIGHT];
    else
        dist_back = IRDIST_MAX;

    switch(sm_dab)
    {
        case 0:

            if(dist_back != IRDIST_MAX)//maze_getWall(&robot.pos, robot.dir+2) > MAZE_ISWALL) //There IS a wall behind the robot
            {
                timer_alignBack = timer;
                sm_dab = 1;
            }
            else
                sm_dab = 2;

            break;

        case 1:
            steer_dab = ((dist_to - dist_back) * KP_ALIGN_BACK);

            if((abs(steer_dab) <= STEER_ALIGN_BACK_END) || ((timer - timer_alignBack) > TIMER_ALIGN_BACK) ||
                (dist_back > TILE1_BACK_TH_BACK))
            {
                mot.d[LEFT].speed.to = 0;
                mot.d[RIGHT].speed.to = 0;
                sm_dab = 2;
            }
            else
            {
                mot.d[LEFT].speed.to = steer_dab;
                mot.d[RIGHT].speed.to = steer_dab;

                drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, MAXSPEED);
            }

            break;

        case 2:
            sm_dab = 0;
            returnvar = 0;

    }

    return returnvar;
}

/**
 * \brief drive_turn
 *
 *        Turns the given angle via the drive_rotate function and the
 *        IMU, if the alignment flag in the given data structure is set,
 *        it also aligns to the walls
 *
 * \param t pointer to turn data structure
 */
void drive_turn(D_TURN *t)
{
    switch(t->state)
    {
        case TURN_INIT:
            mot.d[LEFT].speed.to = 0;
            mot.d[RIGHT].speed.to = 0;

            if(mot.d[LEFT].speed.is == 0 && mot.d[RIGHT].speed.is == 0)
            {
                t->state = TURN;
            }

            break;

        case TURN:
            drive_rotate(&t->r);

            if(t->r.state == ROTATE_FINISHED)
            {
                t->r.state = ROTATE_INIT; //Prepare for next rotate

                if(!t->no_align) //Align to walls?
                    t->state = TURN_ALIGN;
                else
                    t->state = TURN_END;

                foutf(&str_debugDrive, "%i: drTrn\n\r", timer);
            }

            break;

        case TURN_ALIGN:

            //Alignment to walls
            if(!drive_align())
            {
                foutf(&str_debugDrive, "%i: drTrn:algn\n\r", timer);

                if((t->newRobDir != NONE) && (maze_getWall(&robot.pos, maze_alignDir(t->newRobDir + 2)) > MAZE_ISWALL))
                {
                    t->state = TURN_ALIGN_BACK;
                }
                else
                {
                    t->state = TURN_END;
                }
            }

            break;

        case TURN_ALIGN_BACK:

            if(!drive_align_back(TILE1_BACK_BACK))
            {
                foutf(&str_debugDrive, "%i: drTrn:algnBack\n\r", timer);
                t->state = TURN_END;
            }

            break;

        case TURN_END:

            t->state = TURN_FINISHED;

        case TURN_FINISHED:

            foutf(&str_debugDrive, "%i: drTrn:end\n\r", timer);

            break;

        default:
            foutf(&str_error, "%i: ERR:sw[drv.03]:DEF\n\r", timer);
            fatal_err = 1;
            t->state = TURN_INIT;
            break;
    }
}

/**
 * \brief drive_ramp
 *
 *        Drive the ramp up or down
 *
 * \param speed speed to drive the ramp. Only tested with positive speed, neg wont work!
 * \param checkpoint_ramp pointer to the checkpoint var: Variable is set to 1 if there is a checkpoint on the ramp and 2 if the checkpoint was within the last 25cm
 * \return 1 if driving, 0 if arrived on the top of the ramp
 */

#define DIST_SOLL_RAMP 60 //distance to the wall

#define KP_RAMP_DIR 0.9
#define KP_RAMP_DIST KP_RAMP_DIR * 0.5

#define TILE1_DIST_FRONT_RAMP 90 //Vorne, IR

int16_t steer_ramp = 0; //Steering var for distance regulation of ramp
int8_t sm_ramp = 0; //Statemachine for ramp
int32_t ramp_checkpoint_enc = 0; //Encoder value for last checkpoint detection/sign
uint16_t ramp_checkpoint_cnt = 0; //Counter for checkpoint detections
int32_t ramp_enc_start = 0; //Encoder value on beginning of ramp

uint8_t drive_ramp(int8_t speed, int8_t *checkpoint_ramp)
{
    uint8_t returnvar = 1;

    switch(sm_ramp)
    {
        case 0:
            sm_ramp = 1;
            ramp_checkpoint_enc = 0;
            ramp_enc_start = mot.enc;

        case 1:

            //////////Speed/distance regulation
            if((dist[LEFT][FRONT] < TILE1_SIDE_TH) &&
                (dist[LEFT][BACK] < TILE1_SIDE_TH))
            {
                if(dist[LEFT][FRONT] < dist[LEFT][BACK])
                    steer_ramp = (((int16_t)(dist[LEFT][BACK] - dist[LEFT][FRONT])) * -KP_RAMP_DIR);
                else
                    steer_ramp = (((int16_t)(dist[LEFT][FRONT] - dist[LEFT][BACK])) * KP_RAMP_DIR);

                steer_ramp += ((int16_t)(DIST_SOLL - dist[LEFT][FRONT]) * -KP_RAMP_DIST);
            }
            else if((dist[RIGHT][FRONT] < TILE1_SIDE_TH) &&
                            (dist[RIGHT][BACK] < TILE1_SIDE_TH))
            {
                if(dist[RIGHT][FRONT] < dist[RIGHT][BACK])
                    steer_ramp = (((int16_t)(dist[RIGHT][BACK] - dist[RIGHT][FRONT])) * KP_RAMP_DIR);
                else
                    steer_ramp = (((int16_t)(dist[RIGHT][FRONT] - dist[RIGHT][BACK])) * -KP_RAMP_DIR);

                steer_ramp += ((int16_t)(DIST_SOLL - dist[RIGHT][FRONT]) * KP_RAMP_DIST);
            }

            mot.d[LEFT].speed.to = (speed - steer_ramp);
            mot.d[RIGHT].speed.to = (speed + steer_ramp);

            drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, 80);

            /////////////Checkpoints/////////////////

            if(checkpoint_ramp != NULL)
            {
                if(groundsens_l < GROUNDSENS_L_TH_CHECKPOINT)
                {
                    if(((mot.enc - ramp_checkpoint_enc) > (1 * ENC_FAC_CM_LR)) ||
                       (ramp_checkpoint_cnt == 0)) //Less than 4 centimeters since last checkpoint detection
                    {
                        ramp_checkpoint_enc = mot.enc;
                        ramp_checkpoint_cnt ++;
                        if(ramp_checkpoint_cnt > GROUNDSENS_CNT_TH_CHECKPOINT) //Detected many checkpoint-looking sings! This is a checkpoint
                        {
                            ramp_checkpoint_cnt = 0;
                            *checkpoint_ramp = 1;
                        }
                    }
                }
                else if((mot.enc - ramp_checkpoint_enc) > (4 * ENC_FAC_CM_LR))
                {
                    ramp_checkpoint_cnt = 0;
                }
            }

            if((um6.isRamp == 0) && //Not at ramp anymore and distance sensors recognized ramp end
               (dist[FRONT][FRONT] < TILE1_DIST_FRONT_RAMP) &&
                (dist[FRONT][LEFT] < TILE1_DIST_FRONT_RAMP))
            {
                if((checkpoint_ramp != NULL) &&
                   (mot.enc - ramp_checkpoint_enc) < (25 * ENC_FAC_CM_LR)) //We are now on the end of the ramp and the last checkpoint detection happened within the last 25 cm
                {
                    *checkpoint_ramp = 2; //Means that the checkpoint has to be on the uppder part of the ramp
                }

                returnvar = 0;
                sm_ramp = 0;
                mot.d[LEFT].speed.to = 0;
                mot.d[RIGHT].speed.to = 0;
            }
            break;

        default:
            foutf(&str_error, "%i: ERR:sw[drv.04]:DEF\n\r", timer);
            fatal_err = 1;
            returnvar = 0;
            sm_ramp = 0;
            break;
    }
    return returnvar;
}

/**
 * \brief drive_deployResKit
 *
 *        Deploy rescuekit based on the configuration in the
 *        data struct. This function manages the rotation into the
 *        desired direction (because the robot has only one kit
 *        dropper in the back), the alignments and the turnback.
 * \param dk Pointer to coniguration data structure
 */
void drive_deployResKit(D_DEPLOYKIT *dk)
{
    switch(dk->state)
    {
        case DK_INIT:
            if(dk->amount_to == 0)
            {
                dk->state = DK_END;
                break;
            }

            dk->turn.no_align = 1; //Don`t align on this turn! We want to rotate exactly 90°!
            dk->amount_is = 0;

            dk->alignedToBackwall = 0;

            if(dk->config_dir == LEFT) //Turn 90° left...
            {
                dk->turn.r.angle = 90;
            }
            else //...or right
            {
                dk->turn.r.angle = -90;
            }

            dk->turn.state = TURN_INIT;
            dk->state = DK_TURN_A;

        case DK_TURN_A:

            drive_turn(&dk->turn);

            if(dk->turn.state == TURN_FINISHED)
            {
                dk->turn.state = TURN_INIT;

                dk->state = DK_ALIGN_A;
            }
            break;

        case DK_ALIGN_A:

            if(!drive_align_back(35))
            {
                dk->alignedToBackwall = 1;
                dk->state = DK_DEPL;
            }

            break;

        case DK_DEPL:

            if(dk->amount_is < dk->amount_to)
            {
                if(!rescueKit_drop(2))
                {
                    dk->amount_is++;
                }
            }
            else
            {
                if(dk->alignedToBackwall)
                {
                    dk->state = DK_ALIGN_B;
                }
                else
                {
                    dk->state = DK_CHECK_TURN;
                }
            }

            break;

        case DK_ALIGN_B:

            if(!drive_align_back(TILE1_BACK_BACK))
            {
                dk->state = DK_CHECK_TURN;
            }

            break;

        case DK_CHECK_TURN:

            if(dk->config_turnBack) //Only if we want to turn back
            {
                dk->state = DK_TURN_B;

                if(dk->config_dir == LEFT) //Turn back
                {
                    dk->turn.r.angle = -90;
                }
                else
                {
                    dk->turn.r.angle = 90;
                }
            }
            else
            {
                dk->state = DK_END;
            }
            break;

        case DK_TURN_B:

            drive_turn(&dk->turn);

            if(dk->turn.state == TURN_FINISHED)
            {
                dk->turn.state = TURN_INIT;

                dk->state = DK_END;
            }
            break;

        case DK_END:
            dk->state = DK_FINISHED;

        case DK_FINISHED:
            break;

    }
}

///////////////////////////////////////
uint8_t sm_d_lr = 0;//drive_leftright

#define DRIVE_LEFT_FAC 0.8
#define DRIVE_RIGHT_FAC 0.95

/**
 * \brief drive_lr
 *
 *        This is an experimental function to perform driving left or right without turning.
 *
 * \param left goto left direction? If 0, goto right direction.
 * \param speed Speed to drive
 * \param width distance to drive
 * \return 1 if driving, 0 if done
 */
uint8_t drive_lr(int8_t left, int8_t speed, uint8_t width) //left == 1: links, sonst rechts
{
    uint8_t returnvar = 1;
    uint8_t width_var = width;

    switch(sm_d_lr)
    {
        case 0:
            if(left)
                sm_d_lr = 1;
            else
                sm_d_lr = 2;

            foutf(&str_debugDrive, "%i: drLR\n\r", timer);
            break;
        case 1:
            if(!(drive_dist(1, speed, -width)))
            {
                if(left)
                    sm_d_lr = 2;
                else
                    sm_d_lr = 4;
            }
            break;
        case 2:
            if(!(drive_dist(-1, speed, -width)))
            {
                if(left)
                    sm_d_lr = 3;
                else
                    sm_d_lr = 1;
            }
            break;
        case 3:
            if(!(left))
                width_var *= DRIVE_RIGHT_FAC;

            if(!(drive_dist(1, speed, width_var)))
            {
                if(left)
                    sm_d_lr = 4;
                else
                    sm_d_lr = 5;
            }
            break;
        case 4:
            if(left)
                width_var *= DRIVE_LEFT_FAC;

            if(!(drive_dist(-1, speed, width_var)))
            {
                if(left)
                    sm_d_lr = 5;
                else
                    sm_d_lr = 3;
            }
            break;
        case 5:
            sm_d_lr = 0;
            returnvar = 0;

            foutf(&str_debugDrive, "%i: drLR:end\n\r", timer);
            break;
        default:
            foutf(&str_error, "%i: ERR:sw[drv.05]:DEF\n\r", timer);
            fatal_err = 1;
            returnvar = 0;
            break;
    }
    return returnvar;
}

/**
 * \brief drive_dist
 *        Drive a given distance by odometry.
 *        Instanciation not implemented, yet.
 * \param motor motor to move: < 0 ~ left, 0 ~ both, > 0 ~ right
 * \param speed Speed
 * \param dist_cm Distance to drive
 * \return 1 if driving, 0 if done
 */
uint8_t sm_ddist = 0; //state machine

int32_t enc_l_start_ddist = 0;
int32_t enc_r_start_ddist = 0;

uint32_t drive_dist_timer = 0;

uint8_t drive_dist(int8_t motor, int8_t speed, int8_t dist_cm)
{
    uint8_t returnvar = 1;

    switch(sm_ddist)
    {
        case 0:
            enc_l_start_ddist = mot.d[LEFT].enc;
            enc_r_start_ddist = mot.d[RIGHT].enc;

            sm_ddist = 1;

            drive_dist_timer = timer;

            foutf(&str_debugDrive, "%i: drDst\n\r", timer);
            break;
        case 1:
            if(dist_cm < 0)
            {
                if(motor < 0)
                {
                    if(mot.d[LEFT].enc > (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) //Ziel erreicht?
                    {
                        mot.d[LEFT].speed.to = -speed;
                        mot.d[RIGHT].speed.to = 0;
                    }
                    else
                    {
                        sm_ddist = 2;
                    }
                }
                else if(motor == 0)
                {
                    if((mot.d[LEFT].enc > (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) || //Ziel erreicht?
                         (mot.d[RIGHT].enc > (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))))
                    {
                        mot.d[LEFT].speed.to = -speed;
                        mot.d[RIGHT].speed.to = -speed;
                    }
                    else
                    {
                        sm_ddist = 2;
                    }
                }
                else if(motor > 0)
                {
                    if(mot.d[RIGHT].enc > (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))) //Ziel erreicht?
                    {
                        mot.d[LEFT].speed.to = 0;
                        mot.d[RIGHT].speed.to = -speed;
                    }
                    else
                    {
                        sm_ddist = 2;
                    }
                }
            }
            else
            {
                if(motor < 0)
                {
                    if(mot.d[LEFT].enc < (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) //Ziel erreicht?
                    {
                        mot.d[LEFT].speed.to = speed;
                        mot.d[RIGHT].speed.to = 0;
                    }
                    else
                    {
                        sm_ddist = 2;
                    }
                }
                else if(motor == 0)
                {
                    if((mot.d[LEFT].enc < (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) || //Ziel erreicht?
                         (mot.d[RIGHT].enc < (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))))
                    {
                        mot.d[LEFT].speed.to = speed;
                        mot.d[RIGHT].speed.to = speed;
                    }
                    else
                    {
                        sm_ddist = 2;
                    }
                }
                else if(motor > 0)
                {
                    if(mot.d[RIGHT].enc < (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))) //Ziel erreicht?
                    {
                        mot.d[LEFT].speed.to = 0;
                        mot.d[RIGHT].speed.to = speed;
                    }
                    else
                    {
                        sm_ddist = 2;
                    }
                }
            }

            if((timer - drive_dist_timer) > 2000)
            {
                foutf(&str_debugDrive, "%i: drDst:abort:time\n\r", timer);
                sm_ddist = 2;
            }
            break;
        case 2:
            sm_ddist = 0;

            mot.d[LEFT].speed.to = 0;
            mot.d[RIGHT].speed.to = 0;

            returnvar = 0;

            foutf(&str_debugDrive, "%i: drDst:end\n\r", timer);
            break;
        default:
            foutf(&str_error, "%i: ERR:sw[drv.06]:DEF\n\r", timer);
            fatal_err = 1;
            returnvar = 0;
            break;
    }
    return returnvar;
}

/**
 * \brief drive_reset
 *
 *        Reset all non-object-orientated driving functions
 */
void drive_reset(void)
{
    sm_driveAlign = 0;
    sm_dab = 0;
    sm_ramp = 0;
    sm_d_lr = 0;
    sm_ddist = 0;

    foutf(&str_debugDrive, "%i: rstDrv\n\r", timer);
}
