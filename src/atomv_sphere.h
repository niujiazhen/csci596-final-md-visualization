/* Sphere geometry copied from the assignment atomv.c, unchanged. */
void makeFastNiceSphere(GLuint listid, double radius) {
/***********************************************************************
Called once to generate and compile sphere geometry into the given
display list id.
***********************************************************************/
  int i,j;
  float lon,lat;
  float loninc,latinc;
  float x,y,z;

  loninc = 2*M_PI/nlon;
  latinc = M_PI/nlat;

  glNewList(listid,GL_COMPILE);

    /* South-pole triangular fan */
    glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0,-1,0);
      glVertex3f(0,-radius,0);
      lon = 0;
      lat = -M_PI/2 + latinc;
      y = sin(lat);
      for (i=0; i<=nlon; i++) {
        x = cos(lon)*cos(lat);
        z = -sin(lon)*cos(lat);
        glNormal3f(x,y,z);
        glVertex3f(x*radius,y*radius,z*radius);
        lon += loninc;
      }
    glEnd();

    /* Quadrilateral stripes to cover the sphere */
    for (j=1; j<nlat-1; j++) {
      lon = 0;
      glBegin(GL_QUAD_STRIP);
        for (i=0; i<=nlon; i++) {
          x = cos(lon)*cos(lat);
          y = sin(lat);
          z = -sin(lon)*cos(lat);
          glNormal3f(x,y,z);
          glVertex3f(x*radius,y*radius,z*radius);
          x = cos(lon)*cos(lat+latinc);
          y = sin(lat+latinc);
          z = -sin(lon)*cos(lat+latinc);
          glNormal3f(x,y,z);
          glVertex3f(x*radius,y*radius,z*radius);
          lon += loninc;
        }
      glEnd();
      lat += latinc;
    }

    /* North-pole triangular fan */
    glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0,1,0);
      glVertex3f(0,radius,0);
      y = sin(lat);
      lon = 0;
      for (i=0; i<=nlon; i++) {
        x = cos(lon)*cos(lat);
        z = -sin(lon)*cos(lat);
        glNormal3f(x,y,z);
        glVertex3f(x*radius,y*radius,z*radius);
        lon += loninc;
      }
    glEnd();

  glEndList();
}

/**********************************************************************/

