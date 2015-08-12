// Test application for determining if a GLX implementation supports multiple
// GLXPixmaps bound to the same Pixmap out of process.

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <stdio.h>

static int verbose;

GLXPixmap
createGLXPixmap(Display* display, Pixmap pixmap)
{
  const int attribs[] = {
      GLX_RENDER_TYPE, GLX_RGBA_BIT,
      GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
      None
  };

  int numConfigs = 0;
  GLXFBConfig* config = glXChooseFBConfig(display, 0, attribs, &numConfigs);

  if (numConfigs == 0) {
    fprintf(stderr, "Failed to find a valid GLX FBConfig.\n");
    return None;
  }

  GLXFBConfig cfg = config[0];

  if (verbose) {
    GLXFBConfigID id;
    glXGetFBConfigAttrib(display, cfg, GLX_FBCONFIG_ID, (int*) &id);
    printf("Using FBConfig with ID 0x%X\n", id);
  }

  return glXCreatePixmap(display, cfg, pixmap, NULL);
}

int
main(int argc, char* argv[])
{
  verbose = !!getenv("VERBOSE");

  Display* display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Failed to open parent display.\n");
    return 1;
  }

  Window root = XDefaultRootWindow(display);
  Pixmap pix = XCreatePixmap(display, root, 1, 1, 24);

  GLXPixmap parentPix = None;
  if ((parentPix = createGLXPixmap(display, pix)) == None) {
    fprintf(stderr, "Failed to create GLXPixmap in parent!\n");
  }

  if (verbose)
    printf("Created parent GLXPixmap %x\n", parentPix);

  if (fork() == 0) {
    Display* childDisplay = XOpenDisplay(NULL);
    if (!childDisplay) {
      fprintf(stderr, "Failed to open child display.\n");
      return 1;
    }

    GLXPixmap childPix = None;
    if ((childPix = createGLXPixmap(childDisplay, pix)) == None) {
      fprintf(stderr, "Failed to create child pixmap!\n");
      return 1;
    }

    if (verbose)
      printf("Created child GLXPixmap %x\n", childPix);
  } else {
    int status;
    wait(&status);
    if (status == 1) {
      fprintf(stderr, "Test failed; your driver likely does not support multiple binding with GLXPixmaps.\n");
      return 1;
    }
    if (verbose)
      printf("Successfully created parent and child GLXPixmaps; your driver rocks!\n");
  }
  return 0;
}
