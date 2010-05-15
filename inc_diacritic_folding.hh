// This file was generated automatically. Do not edit directly.

enum { foldDiacriticMaxIn = 3 };

wchar foldDiacritic( wchar const * in, size_t size, size_t & consumed )
{
  if ( size > 0 )
  {
    switch( in[ 0 ] )
    {
      case 0x41:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x41;
            case 0x301:
              consumed = 2; return 0x41;
            case 0x302:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x41;
                  case 0x301:
                    consumed = 3; return 0x41;
                  case 0x303:
                    consumed = 3; return 0x41;
                  case 0x309:
                    consumed = 3; return 0x41;
                }
              }
              consumed = 2; return 0x41;
            case 0x303:
              consumed = 2; return 0x41;
            case 0x304:
              consumed = 2; return 0x41;
            case 0x306:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x41;
                  case 0x301:
                    consumed = 3; return 0x41;
                  case 0x303:
                    consumed = 3; return 0x41;
                  case 0x309:
                    consumed = 3; return 0x41;
                }
              }
              consumed = 2; return 0x41;
            case 0x307:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x41;
                }
              }
              consumed = 2; return 0x41;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x41;
                }
              }
              consumed = 2; return 0x41;
            case 0x309:
              consumed = 2; return 0x41;
            case 0x30a:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x41;
                }
              }
              consumed = 2; return 0x41;
            case 0x30c:
              consumed = 2; return 0x41;
            case 0x30f:
              consumed = 2; return 0x41;
            case 0x311:
              consumed = 2; return 0x41;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x302:
                    consumed = 3; return 0x41;
                  case 0x306:
                    consumed = 3; return 0x41;
                }
              }
              consumed = 2; return 0x41;
            case 0x325:
              consumed = 2; return 0x41;
            case 0x328:
              consumed = 2; return 0x41;
          }
        }
        consumed = 1; return *in;
      case 0x42:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x42;
            case 0x323:
              consumed = 2; return 0x42;
            case 0x331:
              consumed = 2; return 0x42;
          }
        }
        consumed = 1; return *in;
      case 0x43:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x43;
            case 0x302:
              consumed = 2; return 0x43;
            case 0x307:
              consumed = 2; return 0x43;
            case 0x30c:
              consumed = 2; return 0x43;
            case 0x327:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x43;
                }
              }
              consumed = 2; return 0x43;
          }
        }
        consumed = 1; return *in;
      case 0x44:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x44;
            case 0x30c:
              consumed = 2; return 0x44;
            case 0x323:
              consumed = 2; return 0x44;
            case 0x327:
              consumed = 2; return 0x44;
            case 0x32d:
              consumed = 2; return 0x44;
            case 0x331:
              consumed = 2; return 0x44;
          }
        }
        consumed = 1; return *in;
      case 0x45:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x45;
            case 0x301:
              consumed = 2; return 0x45;
            case 0x302:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x45;
                  case 0x301:
                    consumed = 3; return 0x45;
                  case 0x303:
                    consumed = 3; return 0x45;
                  case 0x309:
                    consumed = 3; return 0x45;
                }
              }
              consumed = 2; return 0x45;
            case 0x303:
              consumed = 2; return 0x45;
            case 0x304:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x45;
                  case 0x301:
                    consumed = 3; return 0x45;
                }
              }
              consumed = 2; return 0x45;
            case 0x306:
              consumed = 2; return 0x45;
            case 0x307:
              consumed = 2; return 0x45;
            case 0x308:
              consumed = 2; return 0x45;
            case 0x309:
              consumed = 2; return 0x45;
            case 0x30c:
              consumed = 2; return 0x45;
            case 0x30f:
              consumed = 2; return 0x45;
            case 0x311:
              consumed = 2; return 0x45;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x302:
                    consumed = 3; return 0x45;
                }
              }
              consumed = 2; return 0x45;
            case 0x327:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x306:
                    consumed = 3; return 0x45;
                }
              }
              consumed = 2; return 0x45;
            case 0x328:
              consumed = 2; return 0x45;
            case 0x32d:
              consumed = 2; return 0x45;
            case 0x330:
              consumed = 2; return 0x45;
          }
        }
        consumed = 1; return *in;
      case 0x46:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x46;
          }
        }
        consumed = 1; return *in;
      case 0x47:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x47;
            case 0x302:
              consumed = 2; return 0x47;
            case 0x304:
              consumed = 2; return 0x47;
            case 0x306:
              consumed = 2; return 0x47;
            case 0x307:
              consumed = 2; return 0x47;
            case 0x30c:
              consumed = 2; return 0x47;
            case 0x327:
              consumed = 2; return 0x47;
          }
        }
        consumed = 1; return *in;
      case 0x48:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x302:
              consumed = 2; return 0x48;
            case 0x307:
              consumed = 2; return 0x48;
            case 0x308:
              consumed = 2; return 0x48;
            case 0x30c:
              consumed = 2; return 0x48;
            case 0x323:
              consumed = 2; return 0x48;
            case 0x327:
              consumed = 2; return 0x48;
            case 0x32e:
              consumed = 2; return 0x48;
          }
        }
        consumed = 1; return *in;
      case 0x49:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x49;
            case 0x301:
              consumed = 2; return 0x49;
            case 0x302:
              consumed = 2; return 0x49;
            case 0x303:
              consumed = 2; return 0x49;
            case 0x304:
              consumed = 2; return 0x49;
            case 0x306:
              consumed = 2; return 0x49;
            case 0x307:
              consumed = 2; return 0x49;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x49;
                }
              }
              consumed = 2; return 0x49;
            case 0x309:
              consumed = 2; return 0x49;
            case 0x30c:
              consumed = 2; return 0x49;
            case 0x30f:
              consumed = 2; return 0x49;
            case 0x311:
              consumed = 2; return 0x49;
            case 0x323:
              consumed = 2; return 0x49;
            case 0x328:
              consumed = 2; return 0x49;
            case 0x330:
              consumed = 2; return 0x49;
          }
        }
        consumed = 1; return *in;
      case 0x4a:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x302:
              consumed = 2; return 0x4a;
          }
        }
        consumed = 1; return *in;
      case 0x4b:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x4b;
            case 0x30c:
              consumed = 2; return 0x4b;
            case 0x323:
              consumed = 2; return 0x4b;
            case 0x327:
              consumed = 2; return 0x4b;
            case 0x331:
              consumed = 2; return 0x4b;
          }
        }
        consumed = 1; return *in;
      case 0x4c:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x4c;
            case 0x30c:
              consumed = 2; return 0x4c;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x4c;
                }
              }
              consumed = 2; return 0x4c;
            case 0x327:
              consumed = 2; return 0x4c;
            case 0x32d:
              consumed = 2; return 0x4c;
            case 0x331:
              consumed = 2; return 0x4c;
          }
        }
        consumed = 1; return *in;
      case 0x4d:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x4d;
            case 0x307:
              consumed = 2; return 0x4d;
            case 0x323:
              consumed = 2; return 0x4d;
          }
        }
        consumed = 1; return *in;
      case 0x4e:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x4e;
            case 0x301:
              consumed = 2; return 0x4e;
            case 0x303:
              consumed = 2; return 0x4e;
            case 0x307:
              consumed = 2; return 0x4e;
            case 0x30c:
              consumed = 2; return 0x4e;
            case 0x323:
              consumed = 2; return 0x4e;
            case 0x327:
              consumed = 2; return 0x4e;
            case 0x32d:
              consumed = 2; return 0x4e;
            case 0x331:
              consumed = 2; return 0x4e;
          }
        }
        consumed = 1; return *in;
      case 0x4f:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x4f;
            case 0x301:
              consumed = 2; return 0x4f;
            case 0x302:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x4f;
                  case 0x301:
                    consumed = 3; return 0x4f;
                  case 0x303:
                    consumed = 3; return 0x4f;
                  case 0x309:
                    consumed = 3; return 0x4f;
                }
              }
              consumed = 2; return 0x4f;
            case 0x303:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x4f;
                  case 0x304:
                    consumed = 3; return 0x4f;
                  case 0x308:
                    consumed = 3; return 0x4f;
                }
              }
              consumed = 2; return 0x4f;
            case 0x304:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x4f;
                  case 0x301:
                    consumed = 3; return 0x4f;
                }
              }
              consumed = 2; return 0x4f;
            case 0x306:
              consumed = 2; return 0x4f;
            case 0x307:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x4f;
                }
              }
              consumed = 2; return 0x4f;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x4f;
                }
              }
              consumed = 2; return 0x4f;
            case 0x309:
              consumed = 2; return 0x4f;
            case 0x30b:
              consumed = 2; return 0x4f;
            case 0x30c:
              consumed = 2; return 0x4f;
            case 0x30f:
              consumed = 2; return 0x4f;
            case 0x311:
              consumed = 2; return 0x4f;
            case 0x31b:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x4f;
                  case 0x301:
                    consumed = 3; return 0x4f;
                  case 0x303:
                    consumed = 3; return 0x4f;
                  case 0x309:
                    consumed = 3; return 0x4f;
                  case 0x323:
                    consumed = 3; return 0x4f;
                }
              }
              consumed = 2; return 0x4f;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x302:
                    consumed = 3; return 0x4f;
                }
              }
              consumed = 2; return 0x4f;
            case 0x328:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x4f;
                }
              }
              consumed = 2; return 0x4f;
          }
        }
        consumed = 1; return *in;
      case 0x50:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x50;
            case 0x307:
              consumed = 2; return 0x50;
          }
        }
        consumed = 1; return *in;
      case 0x52:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x52;
            case 0x307:
              consumed = 2; return 0x52;
            case 0x30c:
              consumed = 2; return 0x52;
            case 0x30f:
              consumed = 2; return 0x52;
            case 0x311:
              consumed = 2; return 0x52;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x52;
                }
              }
              consumed = 2; return 0x52;
            case 0x327:
              consumed = 2; return 0x52;
            case 0x331:
              consumed = 2; return 0x52;
          }
        }
        consumed = 1; return *in;
      case 0x53:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x307:
                    consumed = 3; return 0x53;
                }
              }
              consumed = 2; return 0x53;
            case 0x302:
              consumed = 2; return 0x53;
            case 0x307:
              consumed = 2; return 0x53;
            case 0x30c:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x307:
                    consumed = 3; return 0x53;
                }
              }
              consumed = 2; return 0x53;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x307:
                    consumed = 3; return 0x53;
                }
              }
              consumed = 2; return 0x53;
            case 0x326:
              consumed = 2; return 0x53;
            case 0x327:
              consumed = 2; return 0x53;
          }
        }
        consumed = 1; return *in;
      case 0x54:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x54;
            case 0x30c:
              consumed = 2; return 0x54;
            case 0x323:
              consumed = 2; return 0x54;
            case 0x326:
              consumed = 2; return 0x54;
            case 0x327:
              consumed = 2; return 0x54;
            case 0x32d:
              consumed = 2; return 0x54;
            case 0x331:
              consumed = 2; return 0x54;
          }
        }
        consumed = 1; return *in;
      case 0x55:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x55;
            case 0x301:
              consumed = 2; return 0x55;
            case 0x302:
              consumed = 2; return 0x55;
            case 0x303:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x55;
                }
              }
              consumed = 2; return 0x55;
            case 0x304:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x308:
                    consumed = 3; return 0x55;
                }
              }
              consumed = 2; return 0x55;
            case 0x306:
              consumed = 2; return 0x55;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x55;
                  case 0x301:
                    consumed = 3; return 0x55;
                  case 0x304:
                    consumed = 3; return 0x55;
                  case 0x30c:
                    consumed = 3; return 0x55;
                }
              }
              consumed = 2; return 0x55;
            case 0x309:
              consumed = 2; return 0x55;
            case 0x30a:
              consumed = 2; return 0x55;
            case 0x30b:
              consumed = 2; return 0x55;
            case 0x30c:
              consumed = 2; return 0x55;
            case 0x30f:
              consumed = 2; return 0x55;
            case 0x311:
              consumed = 2; return 0x55;
            case 0x31b:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x55;
                  case 0x301:
                    consumed = 3; return 0x55;
                  case 0x303:
                    consumed = 3; return 0x55;
                  case 0x309:
                    consumed = 3; return 0x55;
                  case 0x323:
                    consumed = 3; return 0x55;
                }
              }
              consumed = 2; return 0x55;
            case 0x323:
              consumed = 2; return 0x55;
            case 0x324:
              consumed = 2; return 0x55;
            case 0x328:
              consumed = 2; return 0x55;
            case 0x32d:
              consumed = 2; return 0x55;
            case 0x330:
              consumed = 2; return 0x55;
          }
        }
        consumed = 1; return *in;
      case 0x56:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x303:
              consumed = 2; return 0x56;
            case 0x323:
              consumed = 2; return 0x56;
          }
        }
        consumed = 1; return *in;
      case 0x57:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x57;
            case 0x301:
              consumed = 2; return 0x57;
            case 0x302:
              consumed = 2; return 0x57;
            case 0x307:
              consumed = 2; return 0x57;
            case 0x308:
              consumed = 2; return 0x57;
            case 0x323:
              consumed = 2; return 0x57;
          }
        }
        consumed = 1; return *in;
      case 0x58:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x58;
            case 0x308:
              consumed = 2; return 0x58;
          }
        }
        consumed = 1; return *in;
      case 0x59:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x59;
            case 0x301:
              consumed = 2; return 0x59;
            case 0x302:
              consumed = 2; return 0x59;
            case 0x303:
              consumed = 2; return 0x59;
            case 0x304:
              consumed = 2; return 0x59;
            case 0x307:
              consumed = 2; return 0x59;
            case 0x308:
              consumed = 2; return 0x59;
            case 0x309:
              consumed = 2; return 0x59;
            case 0x323:
              consumed = 2; return 0x59;
          }
        }
        consumed = 1; return *in;
      case 0x5a:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x5a;
            case 0x302:
              consumed = 2; return 0x5a;
            case 0x307:
              consumed = 2; return 0x5a;
            case 0x30c:
              consumed = 2; return 0x5a;
            case 0x323:
              consumed = 2; return 0x5a;
            case 0x331:
              consumed = 2; return 0x5a;
          }
        }
        consumed = 1; return *in;
      case 0x61:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x61;
            case 0x301:
              consumed = 2; return 0x61;
            case 0x302:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x61;
                  case 0x301:
                    consumed = 3; return 0x61;
                  case 0x303:
                    consumed = 3; return 0x61;
                  case 0x309:
                    consumed = 3; return 0x61;
                }
              }
              consumed = 2; return 0x61;
            case 0x303:
              consumed = 2; return 0x61;
            case 0x304:
              consumed = 2; return 0x61;
            case 0x306:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x61;
                  case 0x301:
                    consumed = 3; return 0x61;
                  case 0x303:
                    consumed = 3; return 0x61;
                  case 0x309:
                    consumed = 3; return 0x61;
                }
              }
              consumed = 2; return 0x61;
            case 0x307:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x61;
                }
              }
              consumed = 2; return 0x61;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x61;
                }
              }
              consumed = 2; return 0x61;
            case 0x309:
              consumed = 2; return 0x61;
            case 0x30a:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x61;
                }
              }
              consumed = 2; return 0x61;
            case 0x30c:
              consumed = 2; return 0x61;
            case 0x30f:
              consumed = 2; return 0x61;
            case 0x311:
              consumed = 2; return 0x61;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x302:
                    consumed = 3; return 0x61;
                  case 0x306:
                    consumed = 3; return 0x61;
                }
              }
              consumed = 2; return 0x61;
            case 0x325:
              consumed = 2; return 0x61;
            case 0x328:
              consumed = 2; return 0x61;
          }
        }
        consumed = 1; return *in;
      case 0x62:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x62;
            case 0x323:
              consumed = 2; return 0x62;
            case 0x331:
              consumed = 2; return 0x62;
          }
        }
        consumed = 1; return *in;
      case 0x63:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x63;
            case 0x302:
              consumed = 2; return 0x63;
            case 0x307:
              consumed = 2; return 0x63;
            case 0x30c:
              consumed = 2; return 0x63;
            case 0x327:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x63;
                }
              }
              consumed = 2; return 0x63;
          }
        }
        consumed = 1; return *in;
      case 0x64:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x64;
            case 0x30c:
              consumed = 2; return 0x64;
            case 0x323:
              consumed = 2; return 0x64;
            case 0x327:
              consumed = 2; return 0x64;
            case 0x32d:
              consumed = 2; return 0x64;
            case 0x331:
              consumed = 2; return 0x64;
          }
        }
        consumed = 1; return *in;
      case 0x65:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x65;
            case 0x301:
              consumed = 2; return 0x65;
            case 0x302:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x65;
                  case 0x301:
                    consumed = 3; return 0x65;
                  case 0x303:
                    consumed = 3; return 0x65;
                  case 0x309:
                    consumed = 3; return 0x65;
                }
              }
              consumed = 2; return 0x65;
            case 0x303:
              consumed = 2; return 0x65;
            case 0x304:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x65;
                  case 0x301:
                    consumed = 3; return 0x65;
                }
              }
              consumed = 2; return 0x65;
            case 0x306:
              consumed = 2; return 0x65;
            case 0x307:
              consumed = 2; return 0x65;
            case 0x308:
              consumed = 2; return 0x65;
            case 0x309:
              consumed = 2; return 0x65;
            case 0x30c:
              consumed = 2; return 0x65;
            case 0x30f:
              consumed = 2; return 0x65;
            case 0x311:
              consumed = 2; return 0x65;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x302:
                    consumed = 3; return 0x65;
                }
              }
              consumed = 2; return 0x65;
            case 0x327:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x306:
                    consumed = 3; return 0x65;
                }
              }
              consumed = 2; return 0x65;
            case 0x328:
              consumed = 2; return 0x65;
            case 0x32d:
              consumed = 2; return 0x65;
            case 0x330:
              consumed = 2; return 0x65;
          }
        }
        consumed = 1; return *in;
      case 0x66:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x66;
          }
        }
        consumed = 1; return *in;
      case 0x67:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x67;
            case 0x302:
              consumed = 2; return 0x67;
            case 0x304:
              consumed = 2; return 0x67;
            case 0x306:
              consumed = 2; return 0x67;
            case 0x307:
              consumed = 2; return 0x67;
            case 0x30c:
              consumed = 2; return 0x67;
            case 0x327:
              consumed = 2; return 0x67;
          }
        }
        consumed = 1; return *in;
      case 0x68:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x302:
              consumed = 2; return 0x68;
            case 0x307:
              consumed = 2; return 0x68;
            case 0x308:
              consumed = 2; return 0x68;
            case 0x30c:
              consumed = 2; return 0x68;
            case 0x323:
              consumed = 2; return 0x68;
            case 0x327:
              consumed = 2; return 0x68;
            case 0x32e:
              consumed = 2; return 0x68;
            case 0x331:
              consumed = 2; return 0x68;
          }
        }
        consumed = 1; return *in;
      case 0x69:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x69;
            case 0x301:
              consumed = 2; return 0x69;
            case 0x302:
              consumed = 2; return 0x69;
            case 0x303:
              consumed = 2; return 0x69;
            case 0x304:
              consumed = 2; return 0x69;
            case 0x306:
              consumed = 2; return 0x69;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x69;
                }
              }
              consumed = 2; return 0x69;
            case 0x309:
              consumed = 2; return 0x69;
            case 0x30c:
              consumed = 2; return 0x69;
            case 0x30f:
              consumed = 2; return 0x69;
            case 0x311:
              consumed = 2; return 0x69;
            case 0x323:
              consumed = 2; return 0x69;
            case 0x328:
              consumed = 2; return 0x69;
            case 0x330:
              consumed = 2; return 0x69;
          }
        }
        consumed = 1; return *in;
      case 0x6a:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x302:
              consumed = 2; return 0x6a;
            case 0x30c:
              consumed = 2; return 0x6a;
          }
        }
        consumed = 1; return *in;
      case 0x6b:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x6b;
            case 0x30c:
              consumed = 2; return 0x6b;
            case 0x323:
              consumed = 2; return 0x6b;
            case 0x327:
              consumed = 2; return 0x6b;
            case 0x331:
              consumed = 2; return 0x6b;
          }
        }
        consumed = 1; return *in;
      case 0x6c:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x6c;
            case 0x30c:
              consumed = 2; return 0x6c;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x6c;
                }
              }
              consumed = 2; return 0x6c;
            case 0x327:
              consumed = 2; return 0x6c;
            case 0x32d:
              consumed = 2; return 0x6c;
            case 0x331:
              consumed = 2; return 0x6c;
          }
        }
        consumed = 1; return *in;
      case 0x6d:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x6d;
            case 0x307:
              consumed = 2; return 0x6d;
            case 0x323:
              consumed = 2; return 0x6d;
          }
        }
        consumed = 1; return *in;
      case 0x6e:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x6e;
            case 0x301:
              consumed = 2; return 0x6e;
            case 0x303:
              consumed = 2; return 0x6e;
            case 0x307:
              consumed = 2; return 0x6e;
            case 0x30c:
              consumed = 2; return 0x6e;
            case 0x323:
              consumed = 2; return 0x6e;
            case 0x327:
              consumed = 2; return 0x6e;
            case 0x32d:
              consumed = 2; return 0x6e;
            case 0x331:
              consumed = 2; return 0x6e;
          }
        }
        consumed = 1; return *in;
      case 0x6f:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x6f;
            case 0x301:
              consumed = 2; return 0x6f;
            case 0x302:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x6f;
                  case 0x301:
                    consumed = 3; return 0x6f;
                  case 0x303:
                    consumed = 3; return 0x6f;
                  case 0x309:
                    consumed = 3; return 0x6f;
                }
              }
              consumed = 2; return 0x6f;
            case 0x303:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x6f;
                  case 0x304:
                    consumed = 3; return 0x6f;
                  case 0x308:
                    consumed = 3; return 0x6f;
                }
              }
              consumed = 2; return 0x6f;
            case 0x304:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x6f;
                  case 0x301:
                    consumed = 3; return 0x6f;
                }
              }
              consumed = 2; return 0x6f;
            case 0x306:
              consumed = 2; return 0x6f;
            case 0x307:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x6f;
                }
              }
              consumed = 2; return 0x6f;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x6f;
                }
              }
              consumed = 2; return 0x6f;
            case 0x309:
              consumed = 2; return 0x6f;
            case 0x30b:
              consumed = 2; return 0x6f;
            case 0x30c:
              consumed = 2; return 0x6f;
            case 0x30f:
              consumed = 2; return 0x6f;
            case 0x311:
              consumed = 2; return 0x6f;
            case 0x31b:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x6f;
                  case 0x301:
                    consumed = 3; return 0x6f;
                  case 0x303:
                    consumed = 3; return 0x6f;
                  case 0x309:
                    consumed = 3; return 0x6f;
                  case 0x323:
                    consumed = 3; return 0x6f;
                }
              }
              consumed = 2; return 0x6f;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x302:
                    consumed = 3; return 0x6f;
                }
              }
              consumed = 2; return 0x6f;
            case 0x328:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x6f;
                }
              }
              consumed = 2; return 0x6f;
          }
        }
        consumed = 1; return *in;
      case 0x70:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x70;
            case 0x307:
              consumed = 2; return 0x70;
          }
        }
        consumed = 1; return *in;
      case 0x72:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x72;
            case 0x307:
              consumed = 2; return 0x72;
            case 0x30c:
              consumed = 2; return 0x72;
            case 0x30f:
              consumed = 2; return 0x72;
            case 0x311:
              consumed = 2; return 0x72;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x304:
                    consumed = 3; return 0x72;
                }
              }
              consumed = 2; return 0x72;
            case 0x327:
              consumed = 2; return 0x72;
            case 0x331:
              consumed = 2; return 0x72;
          }
        }
        consumed = 1; return *in;
      case 0x73:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x307:
                    consumed = 3; return 0x73;
                }
              }
              consumed = 2; return 0x73;
            case 0x302:
              consumed = 2; return 0x73;
            case 0x307:
              consumed = 2; return 0x73;
            case 0x30c:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x307:
                    consumed = 3; return 0x73;
                }
              }
              consumed = 2; return 0x73;
            case 0x323:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x307:
                    consumed = 3; return 0x73;
                }
              }
              consumed = 2; return 0x73;
            case 0x326:
              consumed = 2; return 0x73;
            case 0x327:
              consumed = 2; return 0x73;
          }
        }
        consumed = 1; return *in;
      case 0x74:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x74;
            case 0x308:
              consumed = 2; return 0x74;
            case 0x30c:
              consumed = 2; return 0x74;
            case 0x323:
              consumed = 2; return 0x74;
            case 0x326:
              consumed = 2; return 0x74;
            case 0x327:
              consumed = 2; return 0x74;
            case 0x32d:
              consumed = 2; return 0x74;
            case 0x331:
              consumed = 2; return 0x74;
          }
        }
        consumed = 1; return *in;
      case 0x75:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x75;
            case 0x301:
              consumed = 2; return 0x75;
            case 0x302:
              consumed = 2; return 0x75;
            case 0x303:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x301:
                    consumed = 3; return 0x75;
                }
              }
              consumed = 2; return 0x75;
            case 0x304:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x308:
                    consumed = 3; return 0x75;
                }
              }
              consumed = 2; return 0x75;
            case 0x306:
              consumed = 2; return 0x75;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x75;
                  case 0x301:
                    consumed = 3; return 0x75;
                  case 0x304:
                    consumed = 3; return 0x75;
                  case 0x30c:
                    consumed = 3; return 0x75;
                }
              }
              consumed = 2; return 0x75;
            case 0x309:
              consumed = 2; return 0x75;
            case 0x30a:
              consumed = 2; return 0x75;
            case 0x30b:
              consumed = 2; return 0x75;
            case 0x30c:
              consumed = 2; return 0x75;
            case 0x30f:
              consumed = 2; return 0x75;
            case 0x311:
              consumed = 2; return 0x75;
            case 0x31b:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x75;
                  case 0x301:
                    consumed = 3; return 0x75;
                  case 0x303:
                    consumed = 3; return 0x75;
                  case 0x309:
                    consumed = 3; return 0x75;
                  case 0x323:
                    consumed = 3; return 0x75;
                }
              }
              consumed = 2; return 0x75;
            case 0x323:
              consumed = 2; return 0x75;
            case 0x324:
              consumed = 2; return 0x75;
            case 0x328:
              consumed = 2; return 0x75;
            case 0x32d:
              consumed = 2; return 0x75;
            case 0x330:
              consumed = 2; return 0x75;
          }
        }
        consumed = 1; return *in;
      case 0x76:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x303:
              consumed = 2; return 0x76;
            case 0x323:
              consumed = 2; return 0x76;
          }
        }
        consumed = 1; return *in;
      case 0x77:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x77;
            case 0x301:
              consumed = 2; return 0x77;
            case 0x302:
              consumed = 2; return 0x77;
            case 0x307:
              consumed = 2; return 0x77;
            case 0x308:
              consumed = 2; return 0x77;
            case 0x30a:
              consumed = 2; return 0x77;
            case 0x323:
              consumed = 2; return 0x77;
          }
        }
        consumed = 1; return *in;
      case 0x78:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x78;
            case 0x308:
              consumed = 2; return 0x78;
          }
        }
        consumed = 1; return *in;
      case 0x79:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x79;
            case 0x301:
              consumed = 2; return 0x79;
            case 0x302:
              consumed = 2; return 0x79;
            case 0x303:
              consumed = 2; return 0x79;
            case 0x304:
              consumed = 2; return 0x79;
            case 0x307:
              consumed = 2; return 0x79;
            case 0x308:
              consumed = 2; return 0x79;
            case 0x309:
              consumed = 2; return 0x79;
            case 0x30a:
              consumed = 2; return 0x79;
            case 0x323:
              consumed = 2; return 0x79;
          }
        }
        consumed = 1; return *in;
      case 0x7a:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x7a;
            case 0x302:
              consumed = 2; return 0x7a;
            case 0x307:
              consumed = 2; return 0x7a;
            case 0x30c:
              consumed = 2; return 0x7a;
            case 0x323:
              consumed = 2; return 0x7a;
            case 0x331:
              consumed = 2; return 0x7a;
          }
        }
        consumed = 1; return *in;
      case 0xc0:
        consumed = 1; return 0x41;
      case 0xc1:
        consumed = 1; return 0x41;
      case 0xc2:
        consumed = 1; return 0x41;
      case 0xc3:
        consumed = 1; return 0x41;
      case 0xc4:
        consumed = 1; return 0x41;
      case 0xc5:
        consumed = 1; return 0x41;
      case 0xc6:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0xc6;
            case 0x304:
              consumed = 2; return 0xc6;
          }
        }
        consumed = 1; return *in;
      case 0xc7:
        consumed = 1; return 0x43;
      case 0xc8:
        consumed = 1; return 0x45;
      case 0xc9:
        consumed = 1; return 0x45;
      case 0xca:
        consumed = 1; return 0x45;
      case 0xcb:
        consumed = 1; return 0x45;
      case 0xcc:
        consumed = 1; return 0x49;
      case 0xcd:
        consumed = 1; return 0x49;
      case 0xce:
        consumed = 1; return 0x49;
      case 0xcf:
        consumed = 1; return 0x49;
      case 0xd1:
        consumed = 1; return 0x4e;
      case 0xd2:
        consumed = 1; return 0x4f;
      case 0xd3:
        consumed = 1; return 0x4f;
      case 0xd4:
        consumed = 1; return 0x4f;
      case 0xd5:
        consumed = 1; return 0x4f;
      case 0xd6:
        consumed = 1; return 0x4f;
      case 0xd8:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0xd8;
          }
        }
        consumed = 1; return 0x4f;
      case 0xd9:
        consumed = 1; return 0x55;
      case 0xda:
        consumed = 1; return 0x55;
      case 0xdb:
        consumed = 1; return 0x55;
      case 0xdc:
        consumed = 1; return 0x55;
      case 0xdd:
        consumed = 1; return 0x59;
      case 0xe0:
        consumed = 1; return 0x61;
      case 0xe1:
        consumed = 1; return 0x61;
      case 0xe2:
        consumed = 1; return 0x61;
      case 0xe3:
        consumed = 1; return 0x61;
      case 0xe4:
        consumed = 1; return 0x61;
      case 0xe5:
        consumed = 1; return 0x61;
      case 0xe6:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0xe6;
            case 0x304:
              consumed = 2; return 0xe6;
          }
        }
        consumed = 1; return *in;
      case 0xe7:
        consumed = 1; return 0x63;
      case 0xe8:
        consumed = 1; return 0x65;
      case 0xe9:
        consumed = 1; return 0x65;
      case 0xea:
        consumed = 1; return 0x65;
      case 0xeb:
        consumed = 1; return 0x65;
      case 0xec:
        consumed = 1; return 0x69;
      case 0xed:
        consumed = 1; return 0x69;
      case 0xee:
        consumed = 1; return 0x69;
      case 0xef:
        consumed = 1; return 0x69;
      case 0xf1:
        consumed = 1; return 0x6e;
      case 0xf2:
        consumed = 1; return 0x6f;
      case 0xf3:
        consumed = 1; return 0x6f;
      case 0xf4:
        consumed = 1; return 0x6f;
      case 0xf5:
        consumed = 1; return 0x6f;
      case 0xf6:
        consumed = 1; return 0x6f;
      case 0xf8:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0xf8;
          }
        }
        consumed = 1; return 0x6f;
      case 0xf9:
        consumed = 1; return 0x75;
      case 0xfa:
        consumed = 1; return 0x75;
      case 0xfb:
        consumed = 1; return 0x75;
      case 0xfc:
        consumed = 1; return 0x75;
      case 0xfd:
        consumed = 1; return 0x79;
      case 0xff:
        consumed = 1; return 0x79;
      case 0x100:
        consumed = 1; return 0x41;
      case 0x101:
        consumed = 1; return 0x61;
      case 0x102:
        consumed = 1; return 0x41;
      case 0x103:
        consumed = 1; return 0x61;
      case 0x104:
        consumed = 1; return 0x41;
      case 0x105:
        consumed = 1; return 0x61;
      case 0x106:
        consumed = 1; return 0x43;
      case 0x107:
        consumed = 1; return 0x63;
      case 0x108:
        consumed = 1; return 0x43;
      case 0x109:
        consumed = 1; return 0x63;
      case 0x10a:
        consumed = 1; return 0x43;
      case 0x10b:
        consumed = 1; return 0x63;
      case 0x10c:
        consumed = 1; return 0x43;
      case 0x10d:
        consumed = 1; return 0x63;
      case 0x10e:
        consumed = 1; return 0x44;
      case 0x10f:
        consumed = 1; return 0x64;
      case 0x110:
        consumed = 1; return 0x44;
      case 0x111:
        consumed = 1; return 0x64;
      case 0x112:
        consumed = 1; return 0x45;
      case 0x113:
        consumed = 1; return 0x65;
      case 0x114:
        consumed = 1; return 0x45;
      case 0x115:
        consumed = 1; return 0x65;
      case 0x116:
        consumed = 1; return 0x45;
      case 0x117:
        consumed = 1; return 0x65;
      case 0x118:
        consumed = 1; return 0x45;
      case 0x119:
        consumed = 1; return 0x65;
      case 0x11a:
        consumed = 1; return 0x45;
      case 0x11b:
        consumed = 1; return 0x65;
      case 0x11c:
        consumed = 1; return 0x47;
      case 0x11d:
        consumed = 1; return 0x67;
      case 0x11e:
        consumed = 1; return 0x47;
      case 0x11f:
        consumed = 1; return 0x67;
      case 0x120:
        consumed = 1; return 0x47;
      case 0x121:
        consumed = 1; return 0x67;
      case 0x122:
        consumed = 1; return 0x47;
      case 0x123:
        consumed = 1; return 0x67;
      case 0x124:
        consumed = 1; return 0x48;
      case 0x125:
        consumed = 1; return 0x68;
      case 0x126:
        consumed = 1; return 0x48;
      case 0x127:
        consumed = 1; return 0x68;
      case 0x128:
        consumed = 1; return 0x49;
      case 0x129:
        consumed = 1; return 0x69;
      case 0x12a:
        consumed = 1; return 0x49;
      case 0x12b:
        consumed = 1; return 0x69;
      case 0x12c:
        consumed = 1; return 0x49;
      case 0x12d:
        consumed = 1; return 0x69;
      case 0x12e:
        consumed = 1; return 0x49;
      case 0x12f:
        consumed = 1; return 0x69;
      case 0x130:
        consumed = 1; return 0x49;
      case 0x134:
        consumed = 1; return 0x4a;
      case 0x135:
        consumed = 1; return 0x6a;
      case 0x136:
        consumed = 1; return 0x4b;
      case 0x137:
        consumed = 1; return 0x6b;
      case 0x139:
        consumed = 1; return 0x4c;
      case 0x13a:
        consumed = 1; return 0x6c;
      case 0x13b:
        consumed = 1; return 0x4c;
      case 0x13c:
        consumed = 1; return 0x6c;
      case 0x13d:
        consumed = 1; return 0x4c;
      case 0x13e:
        consumed = 1; return 0x6c;
      case 0x140:
        consumed = 1; return 0x6c;
      case 0x141:
        consumed = 1; return 0x4c;
      case 0x142:
        consumed = 1; return 0x6c;
      case 0x143:
        consumed = 1; return 0x4e;
      case 0x144:
        consumed = 1; return 0x6e;
      case 0x145:
        consumed = 1; return 0x4e;
      case 0x146:
        consumed = 1; return 0x6e;
      case 0x147:
        consumed = 1; return 0x4e;
      case 0x148:
        consumed = 1; return 0x6e;
      case 0x14c:
        consumed = 1; return 0x4f;
      case 0x14d:
        consumed = 1; return 0x6f;
      case 0x14e:
        consumed = 1; return 0x4f;
      case 0x14f:
        consumed = 1; return 0x6f;
      case 0x150:
        consumed = 1; return 0x4f;
      case 0x151:
        consumed = 1; return 0x6f;
      case 0x154:
        consumed = 1; return 0x52;
      case 0x155:
        consumed = 1; return 0x72;
      case 0x156:
        consumed = 1; return 0x52;
      case 0x157:
        consumed = 1; return 0x72;
      case 0x158:
        consumed = 1; return 0x52;
      case 0x159:
        consumed = 1; return 0x72;
      case 0x15a:
        consumed = 1; return 0x53;
      case 0x15b:
        consumed = 1; return 0x73;
      case 0x15c:
        consumed = 1; return 0x53;
      case 0x15d:
        consumed = 1; return 0x73;
      case 0x15e:
        consumed = 1; return 0x53;
      case 0x15f:
        consumed = 1; return 0x73;
      case 0x160:
        consumed = 1; return 0x53;
      case 0x161:
        consumed = 1; return 0x73;
      case 0x162:
        consumed = 1; return 0x54;
      case 0x163:
        consumed = 1; return 0x74;
      case 0x164:
        consumed = 1; return 0x54;
      case 0x165:
        consumed = 1; return 0x74;
      case 0x166:
        consumed = 1; return 0x54;
      case 0x167:
        consumed = 1; return 0x74;
      case 0x168:
        consumed = 1; return 0x55;
      case 0x169:
        consumed = 1; return 0x75;
      case 0x16a:
        consumed = 1; return 0x55;
      case 0x16b:
        consumed = 1; return 0x75;
      case 0x16c:
        consumed = 1; return 0x55;
      case 0x16d:
        consumed = 1; return 0x75;
      case 0x16e:
        consumed = 1; return 0x55;
      case 0x16f:
        consumed = 1; return 0x75;
      case 0x170:
        consumed = 1; return 0x55;
      case 0x171:
        consumed = 1; return 0x75;
      case 0x172:
        consumed = 1; return 0x55;
      case 0x173:
        consumed = 1; return 0x75;
      case 0x174:
        consumed = 1; return 0x57;
      case 0x175:
        consumed = 1; return 0x77;
      case 0x176:
        consumed = 1; return 0x59;
      case 0x177:
        consumed = 1; return 0x79;
      case 0x178:
        consumed = 1; return 0x59;
      case 0x179:
        consumed = 1; return 0x5a;
      case 0x17a:
        consumed = 1; return 0x7a;
      case 0x17b:
        consumed = 1; return 0x5a;
      case 0x17c:
        consumed = 1; return 0x7a;
      case 0x17d:
        consumed = 1; return 0x5a;
      case 0x17e:
        consumed = 1; return 0x7a;
      case 0x17f:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x307:
              consumed = 2; return 0x17f;
          }
        }
        consumed = 1; return *in;
      case 0x180:
        consumed = 1; return 0x62;
      case 0x181:
        consumed = 1; return 0x42;
      case 0x182:
        consumed = 1; return 0x42;
      case 0x183:
        consumed = 1; return 0x62;
      case 0x187:
        consumed = 1; return 0x43;
      case 0x188:
        consumed = 1; return 0x63;
      case 0x18a:
        consumed = 1; return 0x44;
      case 0x18b:
        consumed = 1; return 0x44;
      case 0x18c:
        consumed = 1; return 0x64;
      case 0x191:
        consumed = 1; return 0x46;
      case 0x192:
        consumed = 1; return 0x66;
      case 0x193:
        consumed = 1; return 0x47;
      case 0x197:
        consumed = 1; return 0x49;
      case 0x198:
        consumed = 1; return 0x4b;
      case 0x199:
        consumed = 1; return 0x6b;
      case 0x19a:
        consumed = 1; return 0x6c;
      case 0x19d:
        consumed = 1; return 0x4e;
      case 0x19e:
        consumed = 1; return 0x6e;
      case 0x19f:
        consumed = 1; return 0x4f;
      case 0x1a0:
        consumed = 1; return 0x4f;
      case 0x1a1:
        consumed = 1; return 0x6f;
      case 0x1a4:
        consumed = 1; return 0x50;
      case 0x1a5:
        consumed = 1; return 0x70;
      case 0x1ab:
        consumed = 1; return 0x74;
      case 0x1ac:
        consumed = 1; return 0x54;
      case 0x1ad:
        consumed = 1; return 0x74;
      case 0x1ae:
        consumed = 1; return 0x54;
      case 0x1af:
        consumed = 1; return 0x55;
      case 0x1b0:
        consumed = 1; return 0x75;
      case 0x1b2:
        consumed = 1; return 0x56;
      case 0x1b3:
        consumed = 1; return 0x59;
      case 0x1b4:
        consumed = 1; return 0x79;
      case 0x1b5:
        consumed = 1; return 0x5a;
      case 0x1b6:
        consumed = 1; return 0x7a;
      case 0x1b7:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x30c:
              consumed = 2; return 0x1b7;
          }
        }
        consumed = 1; return *in;
      case 0x1ba:
        consumed = 1; return 0x292;
      case 0x1cd:
        consumed = 1; return 0x41;
      case 0x1ce:
        consumed = 1; return 0x61;
      case 0x1cf:
        consumed = 1; return 0x49;
      case 0x1d0:
        consumed = 1; return 0x69;
      case 0x1d1:
        consumed = 1; return 0x4f;
      case 0x1d2:
        consumed = 1; return 0x6f;
      case 0x1d3:
        consumed = 1; return 0x55;
      case 0x1d4:
        consumed = 1; return 0x75;
      case 0x1d5:
        consumed = 1; return 0x55;
      case 0x1d6:
        consumed = 1; return 0x75;
      case 0x1d7:
        consumed = 1; return 0x55;
      case 0x1d8:
        consumed = 1; return 0x75;
      case 0x1d9:
        consumed = 1; return 0x55;
      case 0x1da:
        consumed = 1; return 0x75;
      case 0x1db:
        consumed = 1; return 0x55;
      case 0x1dc:
        consumed = 1; return 0x75;
      case 0x1de:
        consumed = 1; return 0x41;
      case 0x1df:
        consumed = 1; return 0x61;
      case 0x1e0:
        consumed = 1; return 0x41;
      case 0x1e1:
        consumed = 1; return 0x61;
      case 0x1e2:
        consumed = 1; return 0xc6;
      case 0x1e3:
        consumed = 1; return 0xe6;
      case 0x1e4:
        consumed = 1; return 0x47;
      case 0x1e5:
        consumed = 1; return 0x67;
      case 0x1e6:
        consumed = 1; return 0x47;
      case 0x1e7:
        consumed = 1; return 0x67;
      case 0x1e8:
        consumed = 1; return 0x4b;
      case 0x1e9:
        consumed = 1; return 0x6b;
      case 0x1ea:
        consumed = 1; return 0x4f;
      case 0x1eb:
        consumed = 1; return 0x6f;
      case 0x1ec:
        consumed = 1; return 0x4f;
      case 0x1ed:
        consumed = 1; return 0x6f;
      case 0x1ee:
        consumed = 1; return 0x1b7;
      case 0x1ef:
        consumed = 1; return 0x292;
      case 0x1f0:
        consumed = 1; return 0x6a;
      case 0x1f4:
        consumed = 1; return 0x47;
      case 0x1f5:
        consumed = 1; return 0x67;
      case 0x1f8:
        consumed = 1; return 0x4e;
      case 0x1f9:
        consumed = 1; return 0x6e;
      case 0x1fa:
        consumed = 1; return 0x41;
      case 0x1fb:
        consumed = 1; return 0x61;
      case 0x1fc:
        consumed = 1; return 0xc6;
      case 0x1fd:
        consumed = 1; return 0xe6;
      case 0x1fe:
        consumed = 1; return 0x4f;
      case 0x1ff:
        consumed = 1; return 0x6f;
      case 0x200:
        consumed = 1; return 0x41;
      case 0x201:
        consumed = 1; return 0x61;
      case 0x202:
        consumed = 1; return 0x41;
      case 0x203:
        consumed = 1; return 0x61;
      case 0x204:
        consumed = 1; return 0x45;
      case 0x205:
        consumed = 1; return 0x65;
      case 0x206:
        consumed = 1; return 0x45;
      case 0x207:
        consumed = 1; return 0x65;
      case 0x208:
        consumed = 1; return 0x49;
      case 0x209:
        consumed = 1; return 0x69;
      case 0x20a:
        consumed = 1; return 0x49;
      case 0x20b:
        consumed = 1; return 0x69;
      case 0x20c:
        consumed = 1; return 0x4f;
      case 0x20d:
        consumed = 1; return 0x6f;
      case 0x20e:
        consumed = 1; return 0x4f;
      case 0x20f:
        consumed = 1; return 0x6f;
      case 0x210:
        consumed = 1; return 0x52;
      case 0x211:
        consumed = 1; return 0x72;
      case 0x212:
        consumed = 1; return 0x52;
      case 0x213:
        consumed = 1; return 0x72;
      case 0x214:
        consumed = 1; return 0x55;
      case 0x215:
        consumed = 1; return 0x75;
      case 0x216:
        consumed = 1; return 0x55;
      case 0x217:
        consumed = 1; return 0x75;
      case 0x218:
        consumed = 1; return 0x53;
      case 0x219:
        consumed = 1; return 0x73;
      case 0x21a:
        consumed = 1; return 0x54;
      case 0x21b:
        consumed = 1; return 0x74;
      case 0x21e:
        consumed = 1; return 0x48;
      case 0x21f:
        consumed = 1; return 0x68;
      case 0x220:
        consumed = 1; return 0x4e;
      case 0x221:
        consumed = 1; return 0x64;
      case 0x224:
        consumed = 1; return 0x5a;
      case 0x225:
        consumed = 1; return 0x7a;
      case 0x226:
        consumed = 1; return 0x41;
      case 0x227:
        consumed = 1; return 0x61;
      case 0x228:
        consumed = 1; return 0x45;
      case 0x229:
        consumed = 1; return 0x65;
      case 0x22a:
        consumed = 1; return 0x4f;
      case 0x22b:
        consumed = 1; return 0x6f;
      case 0x22c:
        consumed = 1; return 0x4f;
      case 0x22d:
        consumed = 1; return 0x6f;
      case 0x22e:
        consumed = 1; return 0x4f;
      case 0x22f:
        consumed = 1; return 0x6f;
      case 0x230:
        consumed = 1; return 0x4f;
      case 0x231:
        consumed = 1; return 0x6f;
      case 0x232:
        consumed = 1; return 0x59;
      case 0x233:
        consumed = 1; return 0x79;
      case 0x234:
        consumed = 1; return 0x6c;
      case 0x235:
        consumed = 1; return 0x6e;
      case 0x236:
        consumed = 1; return 0x74;
      case 0x253:
        consumed = 1; return 0x62;
      case 0x255:
        consumed = 1; return 0x63;
      case 0x256:
        consumed = 1; return 0x64;
      case 0x257:
        consumed = 1; return 0x64;
      case 0x25a:
        consumed = 1; return 0x259;
      case 0x260:
        consumed = 1; return 0x67;
      case 0x266:
        consumed = 1; return 0x68;
      case 0x268:
        consumed = 1; return 0x69;
      case 0x26b:
        consumed = 1; return 0x6c;
      case 0x26c:
        consumed = 1; return 0x6c;
      case 0x26d:
        consumed = 1; return 0x6c;
      case 0x271:
        consumed = 1; return 0x6d;
      case 0x272:
        consumed = 1; return 0x6e;
      case 0x273:
        consumed = 1; return 0x6e;
      case 0x27c:
        consumed = 1; return 0x72;
      case 0x27d:
        consumed = 1; return 0x72;
      case 0x282:
        consumed = 1; return 0x73;
      case 0x286:
        consumed = 1; return 0x283;
      case 0x288:
        consumed = 1; return 0x74;
      case 0x28b:
        consumed = 1; return 0x76;
      case 0x290:
        consumed = 1; return 0x7a;
      case 0x291:
        consumed = 1; return 0x7a;
      case 0x292:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x30c:
              consumed = 2; return 0x292;
          }
        }
        consumed = 1; return *in;
      case 0x293:
        consumed = 1; return 0x292;
      case 0x29d:
        consumed = 1; return 0x6a;
      case 0x2a0:
        consumed = 1; return 0x71;
      case 0x386:
        consumed = 1; return 0x391;
      case 0x388:
        consumed = 1; return 0x395;
      case 0x389:
        consumed = 1; return 0x397;
      case 0x38a:
        consumed = 1; return 0x399;
      case 0x38c:
        consumed = 1; return 0x39f;
      case 0x38e:
        consumed = 1; return 0x3a5;
      case 0x38f:
        consumed = 1; return 0x3a9;
      case 0x390:
        consumed = 1; return 0x3b9;
      case 0x391:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x391;
            case 0x301:
              consumed = 2; return 0x391;
            case 0x304:
              consumed = 2; return 0x391;
            case 0x306:
              consumed = 2; return 0x391;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x391;
                  case 0x301:
                    consumed = 3; return 0x391;
                  case 0x342:
                    consumed = 3; return 0x391;
                }
              }
              consumed = 2; return 0x391;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x391;
                  case 0x301:
                    consumed = 3; return 0x391;
                  case 0x342:
                    consumed = 3; return 0x391;
                }
              }
              consumed = 2; return 0x391;
          }
        }
        consumed = 1; return *in;
      case 0x395:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x395;
            case 0x301:
              consumed = 2; return 0x395;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x395;
                  case 0x301:
                    consumed = 3; return 0x395;
                }
              }
              consumed = 2; return 0x395;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x395;
                  case 0x301:
                    consumed = 3; return 0x395;
                }
              }
              consumed = 2; return 0x395;
          }
        }
        consumed = 1; return *in;
      case 0x397:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x397;
            case 0x301:
              consumed = 2; return 0x397;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x397;
                  case 0x301:
                    consumed = 3; return 0x397;
                  case 0x342:
                    consumed = 3; return 0x397;
                }
              }
              consumed = 2; return 0x397;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x397;
                  case 0x301:
                    consumed = 3; return 0x397;
                  case 0x342:
                    consumed = 3; return 0x397;
                }
              }
              consumed = 2; return 0x397;
          }
        }
        consumed = 1; return *in;
      case 0x399:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x399;
            case 0x301:
              consumed = 2; return 0x399;
            case 0x304:
              consumed = 2; return 0x399;
            case 0x306:
              consumed = 2; return 0x399;
            case 0x308:
              consumed = 2; return 0x399;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x399;
                  case 0x301:
                    consumed = 3; return 0x399;
                  case 0x342:
                    consumed = 3; return 0x399;
                }
              }
              consumed = 2; return 0x399;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x399;
                  case 0x301:
                    consumed = 3; return 0x399;
                  case 0x342:
                    consumed = 3; return 0x399;
                }
              }
              consumed = 2; return 0x399;
          }
        }
        consumed = 1; return *in;
      case 0x39f:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x39f;
            case 0x301:
              consumed = 2; return 0x39f;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x39f;
                  case 0x301:
                    consumed = 3; return 0x39f;
                }
              }
              consumed = 2; return 0x39f;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x39f;
                  case 0x301:
                    consumed = 3; return 0x39f;
                }
              }
              consumed = 2; return 0x39f;
          }
        }
        consumed = 1; return *in;
      case 0x3a1:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x314:
              consumed = 2; return 0x3a1;
          }
        }
        consumed = 1; return *in;
      case 0x3a5:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x3a5;
            case 0x301:
              consumed = 2; return 0x3a5;
            case 0x304:
              consumed = 2; return 0x3a5;
            case 0x306:
              consumed = 2; return 0x3a5;
            case 0x308:
              consumed = 2; return 0x3a5;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3a5;
                  case 0x301:
                    consumed = 3; return 0x3a5;
                  case 0x342:
                    consumed = 3; return 0x3a5;
                }
              }
              consumed = 2; return 0x3a5;
          }
        }
        consumed = 1; return *in;
      case 0x3a9:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x3a9;
            case 0x301:
              consumed = 2; return 0x3a9;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3a9;
                  case 0x301:
                    consumed = 3; return 0x3a9;
                  case 0x342:
                    consumed = 3; return 0x3a9;
                }
              }
              consumed = 2; return 0x3a9;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3a9;
                  case 0x301:
                    consumed = 3; return 0x3a9;
                  case 0x342:
                    consumed = 3; return 0x3a9;
                }
              }
              consumed = 2; return 0x3a9;
          }
        }
        consumed = 1; return *in;
      case 0x3aa:
        consumed = 1; return 0x399;
      case 0x3ab:
        consumed = 1; return 0x3a5;
      case 0x3ac:
        consumed = 1; return 0x3b1;
      case 0x3ad:
        consumed = 1; return 0x3b5;
      case 0x3ae:
        consumed = 1; return 0x3b7;
      case 0x3af:
        consumed = 1; return 0x3b9;
      case 0x3b0:
        consumed = 1; return 0x3c5;
      case 0x3b1:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x3b1;
            case 0x301:
              consumed = 2; return 0x3b1;
            case 0x304:
              consumed = 2; return 0x3b1;
            case 0x306:
              consumed = 2; return 0x3b1;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3b1;
                  case 0x301:
                    consumed = 3; return 0x3b1;
                  case 0x342:
                    consumed = 3; return 0x3b1;
                }
              }
              consumed = 2; return 0x3b1;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3b1;
                  case 0x301:
                    consumed = 3; return 0x3b1;
                  case 0x342:
                    consumed = 3; return 0x3b1;
                }
              }
              consumed = 2; return 0x3b1;
            case 0x342:
              consumed = 2; return 0x3b1;
          }
        }
        consumed = 1; return *in;
      case 0x3b5:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x3b5;
            case 0x301:
              consumed = 2; return 0x3b5;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3b5;
                  case 0x301:
                    consumed = 3; return 0x3b5;
                }
              }
              consumed = 2; return 0x3b5;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3b5;
                  case 0x301:
                    consumed = 3; return 0x3b5;
                }
              }
              consumed = 2; return 0x3b5;
          }
        }
        consumed = 1; return *in;
      case 0x3b7:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x3b7;
            case 0x301:
              consumed = 2; return 0x3b7;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3b7;
                  case 0x301:
                    consumed = 3; return 0x3b7;
                  case 0x342:
                    consumed = 3; return 0x3b7;
                }
              }
              consumed = 2; return 0x3b7;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3b7;
                  case 0x301:
                    consumed = 3; return 0x3b7;
                  case 0x342:
                    consumed = 3; return 0x3b7;
                }
              }
              consumed = 2; return 0x3b7;
            case 0x342:
              consumed = 2; return 0x3b7;
          }
        }
        consumed = 1; return *in;
      case 0x3b9:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x3b9;
            case 0x301:
              consumed = 2; return 0x3b9;
            case 0x304:
              consumed = 2; return 0x3b9;
            case 0x306:
              consumed = 2; return 0x3b9;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3b9;
                  case 0x301:
                    consumed = 3; return 0x3b9;
                  case 0x342:
                    consumed = 3; return 0x3b9;
                }
              }
              consumed = 2; return 0x3b9;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3b9;
                  case 0x301:
                    consumed = 3; return 0x3b9;
                  case 0x342:
                    consumed = 3; return 0x3b9;
                }
              }
              consumed = 2; return 0x3b9;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3b9;
                  case 0x301:
                    consumed = 3; return 0x3b9;
                  case 0x342:
                    consumed = 3; return 0x3b9;
                }
              }
              consumed = 2; return 0x3b9;
            case 0x342:
              consumed = 2; return 0x3b9;
          }
        }
        consumed = 1; return *in;
      case 0x3bf:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x3bf;
            case 0x301:
              consumed = 2; return 0x3bf;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3bf;
                  case 0x301:
                    consumed = 3; return 0x3bf;
                }
              }
              consumed = 2; return 0x3bf;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3bf;
                  case 0x301:
                    consumed = 3; return 0x3bf;
                }
              }
              consumed = 2; return 0x3bf;
          }
        }
        consumed = 1; return *in;
      case 0x3c1:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x313:
              consumed = 2; return 0x3c1;
            case 0x314:
              consumed = 2; return 0x3c1;
          }
        }
        consumed = 1; return *in;
      case 0x3c5:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x3c5;
            case 0x301:
              consumed = 2; return 0x3c5;
            case 0x304:
              consumed = 2; return 0x3c5;
            case 0x306:
              consumed = 2; return 0x3c5;
            case 0x308:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3c5;
                  case 0x301:
                    consumed = 3; return 0x3c5;
                  case 0x342:
                    consumed = 3; return 0x3c5;
                }
              }
              consumed = 2; return 0x3c5;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3c5;
                  case 0x301:
                    consumed = 3; return 0x3c5;
                  case 0x342:
                    consumed = 3; return 0x3c5;
                }
              }
              consumed = 2; return 0x3c5;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3c5;
                  case 0x301:
                    consumed = 3; return 0x3c5;
                  case 0x342:
                    consumed = 3; return 0x3c5;
                }
              }
              consumed = 2; return 0x3c5;
            case 0x342:
              consumed = 2; return 0x3c5;
          }
        }
        consumed = 1; return *in;
      case 0x3c9:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x3c9;
            case 0x301:
              consumed = 2; return 0x3c9;
            case 0x313:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3c9;
                  case 0x301:
                    consumed = 3; return 0x3c9;
                  case 0x342:
                    consumed = 3; return 0x3c9;
                }
              }
              consumed = 2; return 0x3c9;
            case 0x314:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x300:
                    consumed = 3; return 0x3c9;
                  case 0x301:
                    consumed = 3; return 0x3c9;
                  case 0x342:
                    consumed = 3; return 0x3c9;
                }
              }
              consumed = 2; return 0x3c9;
            case 0x342:
              consumed = 2; return 0x3c9;
          }
        }
        consumed = 1; return *in;
      case 0x3ca:
        consumed = 1; return 0x3b9;
      case 0x3cb:
        consumed = 1; return 0x3c5;
      case 0x3cc:
        consumed = 1; return 0x3bf;
      case 0x3cd:
        consumed = 1; return 0x3c5;
      case 0x3ce:
        consumed = 1; return 0x3c9;
      case 0x3d2:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x301:
              consumed = 2; return 0x3d2;
            case 0x308:
              consumed = 2; return 0x3d2;
          }
        }
        consumed = 1; return *in;
      case 0x400:
        consumed = 1; return 0x415;
      case 0x401:
        consumed = 1; return 0x415;
      case 0x40d:
        consumed = 1; return 0x418;
      case 0x410:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x306:
              consumed = 2; return 0x410;
            case 0x308:
              consumed = 2; return 0x410;
          }
        }
        consumed = 1; return *in;
      case 0x415:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x415;
            case 0x306:
              consumed = 2; return 0x415;
          }
        }
        consumed = 1; return *in;
      case 0x416:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x306:
              consumed = 2; return 0x416;
            case 0x308:
              consumed = 2; return 0x416;
          }
        }
        consumed = 1; return *in;
      case 0x417:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x417;
          }
        }
        consumed = 1; return *in;
      case 0x418:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x418;
            case 0x304:
              consumed = 2; return 0x418;
            case 0x308:
              consumed = 2; return 0x418;
          }
        }
        consumed = 1; return *in;
      case 0x41e:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x41e;
          }
        }
        consumed = 1; return *in;
      case 0x423:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x304:
              consumed = 2; return 0x423;
            case 0x308:
              consumed = 2; return 0x423;
            case 0x30b:
              consumed = 2; return 0x423;
          }
        }
        consumed = 1; return *in;
      case 0x427:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x427;
          }
        }
        consumed = 1; return *in;
      case 0x42b:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x42b;
          }
        }
        consumed = 1; return *in;
      case 0x42d:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x42d;
          }
        }
        consumed = 1; return *in;
      case 0x430:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x306:
              consumed = 2; return 0x430;
            case 0x308:
              consumed = 2; return 0x430;
          }
        }
        consumed = 1; return *in;
      case 0x435:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x435;
            case 0x306:
              consumed = 2; return 0x435;
          }
        }
        consumed = 1; return *in;
      case 0x436:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x306:
              consumed = 2; return 0x436;
            case 0x308:
              consumed = 2; return 0x436;
          }
        }
        consumed = 1; return *in;
      case 0x437:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x437;
          }
        }
        consumed = 1; return *in;
      case 0x438:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x300:
              consumed = 2; return 0x438;
            case 0x304:
              consumed = 2; return 0x438;
            case 0x308:
              consumed = 2; return 0x438;
          }
        }
        consumed = 1; return *in;
      case 0x43e:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x43e;
          }
        }
        consumed = 1; return *in;
      case 0x443:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x304:
              consumed = 2; return 0x443;
            case 0x308:
              consumed = 2; return 0x443;
            case 0x30b:
              consumed = 2; return 0x443;
          }
        }
        consumed = 1; return *in;
      case 0x447:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x447;
          }
        }
        consumed = 1; return *in;
      case 0x44b:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x44b;
          }
        }
        consumed = 1; return *in;
      case 0x44d:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x44d;
          }
        }
        consumed = 1; return *in;
      case 0x450:
        consumed = 1; return 0x435;
      case 0x451:
        consumed = 1; return 0x435;
      case 0x45d:
        consumed = 1; return 0x438;
      case 0x474:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x30f:
              consumed = 2; return 0x474;
          }
        }
        consumed = 1; return *in;
      case 0x475:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x30f:
              consumed = 2; return 0x475;
          }
        }
        consumed = 1; return *in;
      case 0x476:
        consumed = 1; return 0x474;
      case 0x477:
        consumed = 1; return 0x475;
      case 0x47c:
        consumed = 1; return 0x460;
      case 0x47d:
        consumed = 1; return 0x461;
      case 0x48a:
        consumed = 1; return 0x419;
      case 0x48b:
        consumed = 1; return 0x439;
      case 0x48e:
        consumed = 1; return 0x420;
      case 0x48f:
        consumed = 1; return 0x440;
      case 0x490:
        consumed = 1; return 0x413;
      case 0x491:
        consumed = 1; return 0x433;
      case 0x492:
        consumed = 1; return 0x413;
      case 0x493:
        consumed = 1; return 0x433;
      case 0x494:
        consumed = 1; return 0x413;
      case 0x495:
        consumed = 1; return 0x433;
      case 0x496:
        consumed = 1; return 0x416;
      case 0x497:
        consumed = 1; return 0x436;
      case 0x498:
        consumed = 1; return 0x417;
      case 0x499:
        consumed = 1; return 0x437;
      case 0x49a:
        consumed = 1; return 0x41a;
      case 0x49b:
        consumed = 1; return 0x43a;
      case 0x49c:
        consumed = 1; return 0x41a;
      case 0x49d:
        consumed = 1; return 0x43a;
      case 0x49e:
        consumed = 1; return 0x41a;
      case 0x49f:
        consumed = 1; return 0x43a;
      case 0x4a2:
        consumed = 1; return 0x41d;
      case 0x4a3:
        consumed = 1; return 0x43d;
      case 0x4a6:
        consumed = 1; return 0x41f;
      case 0x4a7:
        consumed = 1; return 0x43f;
      case 0x4aa:
        consumed = 1; return 0x421;
      case 0x4ab:
        consumed = 1; return 0x441;
      case 0x4ac:
        consumed = 1; return 0x422;
      case 0x4ad:
        consumed = 1; return 0x442;
      case 0x4b0:
        consumed = 1; return 0x4ae;
      case 0x4b1:
        consumed = 1; return 0x4af;
      case 0x4b2:
        consumed = 1; return 0x425;
      case 0x4b3:
        consumed = 1; return 0x425;
      case 0x4b6:
        consumed = 1; return 0x4bc;
      case 0x4b7:
        consumed = 1; return 0x4bc;
      case 0x4b8:
        consumed = 1; return 0x4bc;
      case 0x4b9:
        consumed = 1; return 0x447;
      case 0x4be:
        consumed = 1; return 0x4bc;
      case 0x4bf:
        consumed = 1; return 0x4bc;
      case 0x4c1:
        consumed = 1; return 0x416;
      case 0x4c2:
        consumed = 1; return 0x436;
      case 0x4c3:
        consumed = 1; return 0x41a;
      case 0x4c4:
        consumed = 1; return 0x43a;
      case 0x4c5:
        consumed = 1; return 0x41b;
      case 0x4c6:
        consumed = 1; return 0x43b;
      case 0x4c7:
        consumed = 1; return 0x41d;
      case 0x4c8:
        consumed = 1; return 0x43d;
      case 0x4c9:
        consumed = 1; return 0x41d;
      case 0x4ca:
        consumed = 1; return 0x43d;
      case 0x4cb:
        consumed = 1; return 0x4bc;
      case 0x4cc:
        consumed = 1; return 0x4bc;
      case 0x4cd:
        consumed = 1; return 0x41c;
      case 0x4ce:
        consumed = 1; return 0x43c;
      case 0x4d0:
        consumed = 1; return 0x410;
      case 0x4d1:
        consumed = 1; return 0x430;
      case 0x4d2:
        consumed = 1; return 0x410;
      case 0x4d3:
        consumed = 1; return 0x430;
      case 0x4d6:
        consumed = 1; return 0x415;
      case 0x4d7:
        consumed = 1; return 0x435;
      case 0x4d8:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x4d8;
          }
        }
        consumed = 1; return *in;
      case 0x4d9:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x4d9;
          }
        }
        consumed = 1; return *in;
      case 0x4da:
        consumed = 1; return 0x4d8;
      case 0x4db:
        consumed = 1; return 0x4d9;
      case 0x4dc:
        consumed = 1; return 0x416;
      case 0x4dd:
        consumed = 1; return 0x436;
      case 0x4de:
        consumed = 1; return 0x417;
      case 0x4df:
        consumed = 1; return 0x437;
      case 0x4e2:
        consumed = 1; return 0x418;
      case 0x4e3:
        consumed = 1; return 0x438;
      case 0x4e4:
        consumed = 1; return 0x418;
      case 0x4e5:
        consumed = 1; return 0x438;
      case 0x4e6:
        consumed = 1; return 0x41e;
      case 0x4e7:
        consumed = 1; return 0x43e;
      case 0x4e8:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x4e8;
          }
        }
        consumed = 1; return *in;
      case 0x4e9:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x308:
              consumed = 2; return 0x4e9;
          }
        }
        consumed = 1; return *in;
      case 0x4ea:
        consumed = 1; return 0x4e8;
      case 0x4eb:
        consumed = 1; return 0x4e9;
      case 0x4ec:
        consumed = 1; return 0x42d;
      case 0x4ed:
        consumed = 1; return 0x44d;
      case 0x4ee:
        consumed = 1; return 0x423;
      case 0x4ef:
        consumed = 1; return 0x443;
      case 0x4f0:
        consumed = 1; return 0x423;
      case 0x4f1:
        consumed = 1; return 0x443;
      case 0x4f2:
        consumed = 1; return 0x423;
      case 0x4f3:
        consumed = 1; return 0x443;
      case 0x4f4:
        consumed = 1; return 0x427;
      case 0x4f5:
        consumed = 1; return 0x447;
      case 0x4f8:
        consumed = 1; return 0x42b;
      case 0x4f9:
        consumed = 1; return 0x44b;
      case 0x5d0:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5b7:
              consumed = 2; return 0x5d0;
            case 0x5b8:
              consumed = 2; return 0x5d0;
            case 0x5bc:
              consumed = 2; return 0x5d0;
          }
        }
        consumed = 1; return *in;
      case 0x5d1:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5d1;
            case 0x5bf:
              consumed = 2; return 0x5d1;
          }
        }
        consumed = 1; return *in;
      case 0x5d2:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5d2;
          }
        }
        consumed = 1; return *in;
      case 0x5d3:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5d3;
          }
        }
        consumed = 1; return *in;
      case 0x5d4:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5d4;
          }
        }
        consumed = 1; return *in;
      case 0x5d5:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5b9:
              consumed = 2; return 0x5d5;
            case 0x5bc:
              consumed = 2; return 0x5d5;
          }
        }
        consumed = 1; return *in;
      case 0x5d6:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5d6;
          }
        }
        consumed = 1; return *in;
      case 0x5d8:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5d8;
          }
        }
        consumed = 1; return *in;
      case 0x5d9:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5b4:
              consumed = 2; return 0x5d9;
            case 0x5bc:
              consumed = 2; return 0x5d9;
          }
        }
        consumed = 1; return *in;
      case 0x5da:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5da;
          }
        }
        consumed = 1; return *in;
      case 0x5db:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5db;
            case 0x5bf:
              consumed = 2; return 0x5db;
          }
        }
        consumed = 1; return *in;
      case 0x5dc:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5dc;
          }
        }
        consumed = 1; return *in;
      case 0x5de:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5de;
          }
        }
        consumed = 1; return *in;
      case 0x5e0:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5e0;
          }
        }
        consumed = 1; return *in;
      case 0x5e1:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5e1;
          }
        }
        consumed = 1; return *in;
      case 0x5e3:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5e3;
          }
        }
        consumed = 1; return *in;
      case 0x5e4:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5e4;
            case 0x5bf:
              consumed = 2; return 0x5e4;
          }
        }
        consumed = 1; return *in;
      case 0x5e6:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5e6;
          }
        }
        consumed = 1; return *in;
      case 0x5e7:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5e7;
          }
        }
        consumed = 1; return *in;
      case 0x5e8:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5e8;
          }
        }
        consumed = 1; return *in;
      case 0x5e9:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              if ( size > 2 )
              {
                switch( in[ 2 ] )
                {
                  case 0x5c1:
                    consumed = 3; return 0x5e9;
                  case 0x5c2:
                    consumed = 3; return 0x5e9;
                }
              }
              consumed = 2; return 0x5e9;
            case 0x5c1:
              consumed = 2; return 0x5e9;
            case 0x5c2:
              consumed = 2; return 0x5e9;
          }
        }
        consumed = 1; return *in;
      case 0x5ea:
        if ( size > 1 )
        {
          switch( in[ 1 ] )
          {
            case 0x5bc:
              consumed = 2; return 0x5ea;
          }
        }
        consumed = 1; return *in;
      case 0x1e00:
        consumed = 1; return 0x41;
      case 0x1e01:
        consumed = 1; return 0x61;
      case 0x1e02:
        consumed = 1; return 0x42;
      case 0x1e03:
        consumed = 1; return 0x62;
      case 0x1e04:
        consumed = 1; return 0x42;
      case 0x1e05:
        consumed = 1; return 0x62;
      case 0x1e06:
        consumed = 1; return 0x42;
      case 0x1e07:
        consumed = 1; return 0x62;
      case 0x1e08:
        consumed = 1; return 0x43;
      case 0x1e09:
        consumed = 1; return 0x63;
      case 0x1e0a:
        consumed = 1; return 0x44;
      case 0x1e0b:
        consumed = 1; return 0x64;
      case 0x1e0c:
        consumed = 1; return 0x44;
      case 0x1e0d:
        consumed = 1; return 0x64;
      case 0x1e0e:
        consumed = 1; return 0x44;
      case 0x1e0f:
        consumed = 1; return 0x64;
      case 0x1e10:
        consumed = 1; return 0x44;
      case 0x1e11:
        consumed = 1; return 0x64;
      case 0x1e12:
        consumed = 1; return 0x44;
      case 0x1e13:
        consumed = 1; return 0x64;
      case 0x1e14:
        consumed = 1; return 0x45;
      case 0x1e15:
        consumed = 1; return 0x65;
      case 0x1e16:
        consumed = 1; return 0x45;
      case 0x1e17:
        consumed = 1; return 0x65;
      case 0x1e18:
        consumed = 1; return 0x45;
      case 0x1e19:
        consumed = 1; return 0x65;
      case 0x1e1a:
        consumed = 1; return 0x45;
      case 0x1e1b:
        consumed = 1; return 0x65;
      case 0x1e1c:
        consumed = 1; return 0x45;
      case 0x1e1d:
        consumed = 1; return 0x65;
      case 0x1e1e:
        consumed = 1; return 0x46;
      case 0x1e1f:
        consumed = 1; return 0x66;
      case 0x1e20:
        consumed = 1; return 0x47;
      case 0x1e21:
        consumed = 1; return 0x67;
      case 0x1e22:
        consumed = 1; return 0x48;
      case 0x1e23:
        consumed = 1; return 0x68;
      case 0x1e24:
        consumed = 1; return 0x48;
      case 0x1e25:
        consumed = 1; return 0x68;
      case 0x1e26:
        consumed = 1; return 0x48;
      case 0x1e27:
        consumed = 1; return 0x68;
      case 0x1e28:
        consumed = 1; return 0x48;
      case 0x1e29:
        consumed = 1; return 0x68;
      case 0x1e2a:
        consumed = 1; return 0x48;
      case 0x1e2b:
        consumed = 1; return 0x68;
      case 0x1e2c:
        consumed = 1; return 0x49;
      case 0x1e2d:
        consumed = 1; return 0x69;
      case 0x1e2e:
        consumed = 1; return 0x49;
      case 0x1e2f:
        consumed = 1; return 0x69;
      case 0x1e30:
        consumed = 1; return 0x4b;
      case 0x1e31:
        consumed = 1; return 0x6b;
      case 0x1e32:
        consumed = 1; return 0x4b;
      case 0x1e33:
        consumed = 1; return 0x6b;
      case 0x1e34:
        consumed = 1; return 0x4b;
      case 0x1e35:
        consumed = 1; return 0x6b;
      case 0x1e36:
        consumed = 1; return 0x4c;
      case 0x1e37:
        consumed = 1; return 0x6c;
      case 0x1e38:
        consumed = 1; return 0x4c;
      case 0x1e39:
        consumed = 1; return 0x6c;
      case 0x1e3a:
        consumed = 1; return 0x4c;
      case 0x1e3b:
        consumed = 1; return 0x6c;
      case 0x1e3c:
        consumed = 1; return 0x4c;
      case 0x1e3d:
        consumed = 1; return 0x6c;
      case 0x1e3e:
        consumed = 1; return 0x4d;
      case 0x1e3f:
        consumed = 1; return 0x6d;
      case 0x1e40:
        consumed = 1; return 0x4d;
      case 0x1e41:
        consumed = 1; return 0x6d;
      case 0x1e42:
        consumed = 1; return 0x4d;
      case 0x1e43:
        consumed = 1; return 0x6d;
      case 0x1e44:
        consumed = 1; return 0x4e;
      case 0x1e45:
        consumed = 1; return 0x6e;
      case 0x1e46:
        consumed = 1; return 0x4e;
      case 0x1e47:
        consumed = 1; return 0x6e;
      case 0x1e48:
        consumed = 1; return 0x4e;
      case 0x1e49:
        consumed = 1; return 0x6e;
      case 0x1e4a:
        consumed = 1; return 0x4e;
      case 0x1e4b:
        consumed = 1; return 0x6e;
      case 0x1e4c:
        consumed = 1; return 0x4f;
      case 0x1e4d:
        consumed = 1; return 0x6f;
      case 0x1e4e:
        consumed = 1; return 0x4f;
      case 0x1e4f:
        consumed = 1; return 0x6f;
      case 0x1e50:
        consumed = 1; return 0x4f;
      case 0x1e51:
        consumed = 1; return 0x6f;
      case 0x1e52:
        consumed = 1; return 0x4f;
      case 0x1e53:
        consumed = 1; return 0x6f;
      case 0x1e54:
        consumed = 1; return 0x50;
      case 0x1e55:
        consumed = 1; return 0x70;
      case 0x1e56:
        consumed = 1; return 0x50;
      case 0x1e57:
        consumed = 1; return 0x70;
      case 0x1e58:
        consumed = 1; return 0x52;
      case 0x1e59:
        consumed = 1; return 0x72;
      case 0x1e5a:
        consumed = 1; return 0x52;
      case 0x1e5b:
        consumed = 1; return 0x72;
      case 0x1e5c:
        consumed = 1; return 0x52;
      case 0x1e5d:
        consumed = 1; return 0x72;
      case 0x1e5e:
        consumed = 1; return 0x52;
      case 0x1e5f:
        consumed = 1; return 0x72;
      case 0x1e60:
        consumed = 1; return 0x53;
      case 0x1e61:
        consumed = 1; return 0x73;
      case 0x1e62:
        consumed = 1; return 0x53;
      case 0x1e63:
        consumed = 1; return 0x73;
      case 0x1e64:
        consumed = 1; return 0x53;
      case 0x1e65:
        consumed = 1; return 0x73;
      case 0x1e66:
        consumed = 1; return 0x53;
      case 0x1e67:
        consumed = 1; return 0x73;
      case 0x1e68:
        consumed = 1; return 0x53;
      case 0x1e69:
        consumed = 1; return 0x73;
      case 0x1e6a:
        consumed = 1; return 0x54;
      case 0x1e6b:
        consumed = 1; return 0x74;
      case 0x1e6c:
        consumed = 1; return 0x54;
      case 0x1e6d:
        consumed = 1; return 0x74;
      case 0x1e6e:
        consumed = 1; return 0x54;
      case 0x1e6f:
        consumed = 1; return 0x74;
      case 0x1e70:
        consumed = 1; return 0x54;
      case 0x1e71:
        consumed = 1; return 0x74;
      case 0x1e72:
        consumed = 1; return 0x55;
      case 0x1e73:
        consumed = 1; return 0x75;
      case 0x1e74:
        consumed = 1; return 0x55;
      case 0x1e75:
        consumed = 1; return 0x75;
      case 0x1e76:
        consumed = 1; return 0x55;
      case 0x1e77:
        consumed = 1; return 0x75;
      case 0x1e78:
        consumed = 1; return 0x55;
      case 0x1e79:
        consumed = 1; return 0x75;
      case 0x1e7a:
        consumed = 1; return 0x55;
      case 0x1e7b:
        consumed = 1; return 0x75;
      case 0x1e7c:
        consumed = 1; return 0x56;
      case 0x1e7d:
        consumed = 1; return 0x76;
      case 0x1e7e:
        consumed = 1; return 0x56;
      case 0x1e7f:
        consumed = 1; return 0x76;
      case 0x1e80:
        consumed = 1; return 0x57;
      case 0x1e81:
        consumed = 1; return 0x77;
      case 0x1e82:
        consumed = 1; return 0x57;
      case 0x1e83:
        consumed = 1; return 0x77;
      case 0x1e84:
        consumed = 1; return 0x57;
      case 0x1e85:
        consumed = 1; return 0x77;
      case 0x1e86:
        consumed = 1; return 0x57;
      case 0x1e87:
        consumed = 1; return 0x77;
      case 0x1e88:
        consumed = 1; return 0x57;
      case 0x1e89:
        consumed = 1; return 0x77;
      case 0x1e8a:
        consumed = 1; return 0x58;
      case 0x1e8b:
        consumed = 1; return 0x78;
      case 0x1e8c:
        consumed = 1; return 0x58;
      case 0x1e8d:
        consumed = 1; return 0x78;
      case 0x1e8e:
        consumed = 1; return 0x59;
      case 0x1e8f:
        consumed = 1; return 0x79;
      case 0x1e90:
        consumed = 1; return 0x5a;
      case 0x1e91:
        consumed = 1; return 0x7a;
      case 0x1e92:
        consumed = 1; return 0x5a;
      case 0x1e93:
        consumed = 1; return 0x7a;
      case 0x1e94:
        consumed = 1; return 0x5a;
      case 0x1e95:
        consumed = 1; return 0x7a;
      case 0x1e96:
        consumed = 1; return 0x68;
      case 0x1e97:
        consumed = 1; return 0x74;
      case 0x1e98:
        consumed = 1; return 0x77;
      case 0x1e99:
        consumed = 1; return 0x79;
      case 0x1e9a:
        consumed = 1; return 0x61;
      case 0x1e9b:
        consumed = 1; return 0x17f;
      case 0x1ea0:
        consumed = 1; return 0x41;
      case 0x1ea1:
        consumed = 1; return 0x61;
      case 0x1ea2:
        consumed = 1; return 0x41;
      case 0x1ea3:
        consumed = 1; return 0x61;
      case 0x1ea4:
        consumed = 1; return 0x41;
      case 0x1ea5:
        consumed = 1; return 0x61;
      case 0x1ea6:
        consumed = 1; return 0x41;
      case 0x1ea7:
        consumed = 1; return 0x61;
      case 0x1ea8:
        consumed = 1; return 0x41;
      case 0x1ea9:
        consumed = 1; return 0x61;
      case 0x1eaa:
        consumed = 1; return 0x41;
      case 0x1eab:
        consumed = 1; return 0x61;
      case 0x1eac:
        consumed = 1; return 0x41;
      case 0x1ead:
        consumed = 1; return 0x61;
      case 0x1eae:
        consumed = 1; return 0x41;
      case 0x1eaf:
        consumed = 1; return 0x61;
      case 0x1eb0:
        consumed = 1; return 0x41;
      case 0x1eb1:
        consumed = 1; return 0x61;
      case 0x1eb2:
        consumed = 1; return 0x41;
      case 0x1eb3:
        consumed = 1; return 0x61;
      case 0x1eb4:
        consumed = 1; return 0x41;
      case 0x1eb5:
        consumed = 1; return 0x61;
      case 0x1eb6:
        consumed = 1; return 0x41;
      case 0x1eb7:
        consumed = 1; return 0x61;
      case 0x1eb8:
        consumed = 1; return 0x45;
      case 0x1eb9:
        consumed = 1; return 0x65;
      case 0x1eba:
        consumed = 1; return 0x45;
      case 0x1ebb:
        consumed = 1; return 0x65;
      case 0x1ebc:
        consumed = 1; return 0x45;
      case 0x1ebd:
        consumed = 1; return 0x65;
      case 0x1ebe:
        consumed = 1; return 0x45;
      case 0x1ebf:
        consumed = 1; return 0x65;
      case 0x1ec0:
        consumed = 1; return 0x45;
      case 0x1ec1:
        consumed = 1; return 0x65;
      case 0x1ec2:
        consumed = 1; return 0x45;
      case 0x1ec3:
        consumed = 1; return 0x65;
      case 0x1ec4:
        consumed = 1; return 0x45;
      case 0x1ec5:
        consumed = 1; return 0x65;
      case 0x1ec6:
        consumed = 1; return 0x45;
      case 0x1ec7:
        consumed = 1; return 0x65;
      case 0x1ec8:
        consumed = 1; return 0x49;
      case 0x1ec9:
        consumed = 1; return 0x69;
      case 0x1eca:
        consumed = 1; return 0x49;
      case 0x1ecb:
        consumed = 1; return 0x69;
      case 0x1ecc:
        consumed = 1; return 0x4f;
      case 0x1ecd:
        consumed = 1; return 0x6f;
      case 0x1ece:
        consumed = 1; return 0x4f;
      case 0x1ecf:
        consumed = 1; return 0x6f;
      case 0x1ed0:
        consumed = 1; return 0x4f;
      case 0x1ed1:
        consumed = 1; return 0x6f;
      case 0x1ed2:
        consumed = 1; return 0x4f;
      case 0x1ed3:
        consumed = 1; return 0x6f;
      case 0x1ed4:
        consumed = 1; return 0x4f;
      case 0x1ed5:
        consumed = 1; return 0x6f;
      case 0x1ed6:
        consumed = 1; return 0x4f;
      case 0x1ed7:
        consumed = 1; return 0x6f;
      case 0x1ed8:
        consumed = 1; return 0x4f;
      case 0x1ed9:
        consumed = 1; return 0x6f;
      case 0x1eda:
        consumed = 1; return 0x4f;
      case 0x1edb:
        consumed = 1; return 0x6f;
      case 0x1edc:
        consumed = 1; return 0x4f;
      case 0x1edd:
        consumed = 1; return 0x6f;
      case 0x1ede:
        consumed = 1; return 0x4f;
      case 0x1edf:
        consumed = 1; return 0x6f;
      case 0x1ee0:
        consumed = 1; return 0x4f;
      case 0x1ee1:
        consumed = 1; return 0x6f;
      case 0x1ee2:
        consumed = 1; return 0x4f;
      case 0x1ee3:
        consumed = 1; return 0x6f;
      case 0x1ee4:
        consumed = 1; return 0x55;
      case 0x1ee5:
        consumed = 1; return 0x75;
      case 0x1ee6:
        consumed = 1; return 0x55;
      case 0x1ee7:
        consumed = 1; return 0x75;
      case 0x1ee8:
        consumed = 1; return 0x55;
      case 0x1ee9:
        consumed = 1; return 0x75;
      case 0x1eea:
        consumed = 1; return 0x55;
      case 0x1eeb:
        consumed = 1; return 0x75;
      case 0x1eec:
        consumed = 1; return 0x55;
      case 0x1eed:
        consumed = 1; return 0x75;
      case 0x1eee:
        consumed = 1; return 0x55;
      case 0x1eef:
        consumed = 1; return 0x75;
      case 0x1ef0:
        consumed = 1; return 0x55;
      case 0x1ef1:
        consumed = 1; return 0x75;
      case 0x1ef2:
        consumed = 1; return 0x59;
      case 0x1ef3:
        consumed = 1; return 0x79;
      case 0x1ef4:
        consumed = 1; return 0x59;
      case 0x1ef5:
        consumed = 1; return 0x79;
      case 0x1ef6:
        consumed = 1; return 0x59;
      case 0x1ef7:
        consumed = 1; return 0x79;
      case 0x1ef8:
        consumed = 1; return 0x59;
      case 0x1ef9:
        consumed = 1; return 0x79;
      case 0x1f00:
        consumed = 1; return 0x3b1;
      case 0x1f01:
        consumed = 1; return 0x3b1;
      case 0x1f02:
        consumed = 1; return 0x3b1;
      case 0x1f03:
        consumed = 1; return 0x3b1;
      case 0x1f04:
        consumed = 1; return 0x3b1;
      case 0x1f05:
        consumed = 1; return 0x3b1;
      case 0x1f06:
        consumed = 1; return 0x3b1;
      case 0x1f07:
        consumed = 1; return 0x3b1;
      case 0x1f08:
        consumed = 1; return 0x391;
      case 0x1f09:
        consumed = 1; return 0x391;
      case 0x1f0a:
        consumed = 1; return 0x391;
      case 0x1f0b:
        consumed = 1; return 0x391;
      case 0x1f0c:
        consumed = 1; return 0x391;
      case 0x1f0d:
        consumed = 1; return 0x391;
      case 0x1f0e:
        consumed = 1; return 0x391;
      case 0x1f0f:
        consumed = 1; return 0x391;
      case 0x1f10:
        consumed = 1; return 0x3b5;
      case 0x1f11:
        consumed = 1; return 0x3b5;
      case 0x1f12:
        consumed = 1; return 0x3b5;
      case 0x1f13:
        consumed = 1; return 0x3b5;
      case 0x1f14:
        consumed = 1; return 0x3b5;
      case 0x1f15:
        consumed = 1; return 0x3b5;
      case 0x1f18:
        consumed = 1; return 0x395;
      case 0x1f19:
        consumed = 1; return 0x395;
      case 0x1f1a:
        consumed = 1; return 0x395;
      case 0x1f1b:
        consumed = 1; return 0x395;
      case 0x1f1c:
        consumed = 1; return 0x395;
      case 0x1f1d:
        consumed = 1; return 0x395;
      case 0x1f20:
        consumed = 1; return 0x3b7;
      case 0x1f21:
        consumed = 1; return 0x3b7;
      case 0x1f22:
        consumed = 1; return 0x3b7;
      case 0x1f23:
        consumed = 1; return 0x3b7;
      case 0x1f24:
        consumed = 1; return 0x3b7;
      case 0x1f25:
        consumed = 1; return 0x3b7;
      case 0x1f26:
        consumed = 1; return 0x3b7;
      case 0x1f27:
        consumed = 1; return 0x3b7;
      case 0x1f28:
        consumed = 1; return 0x397;
      case 0x1f29:
        consumed = 1; return 0x397;
      case 0x1f2a:
        consumed = 1; return 0x397;
      case 0x1f2b:
        consumed = 1; return 0x397;
      case 0x1f2c:
        consumed = 1; return 0x397;
      case 0x1f2d:
        consumed = 1; return 0x397;
      case 0x1f2e:
        consumed = 1; return 0x397;
      case 0x1f2f:
        consumed = 1; return 0x397;
      case 0x1f30:
        consumed = 1; return 0x3b9;
      case 0x1f31:
        consumed = 1; return 0x3b9;
      case 0x1f32:
        consumed = 1; return 0x3b9;
      case 0x1f33:
        consumed = 1; return 0x3b9;
      case 0x1f34:
        consumed = 1; return 0x3b9;
      case 0x1f35:
        consumed = 1; return 0x3b9;
      case 0x1f36:
        consumed = 1; return 0x3b9;
      case 0x1f37:
        consumed = 1; return 0x3b9;
      case 0x1f38:
        consumed = 1; return 0x399;
      case 0x1f39:
        consumed = 1; return 0x399;
      case 0x1f3a:
        consumed = 1; return 0x399;
      case 0x1f3b:
        consumed = 1; return 0x399;
      case 0x1f3c:
        consumed = 1; return 0x399;
      case 0x1f3d:
        consumed = 1; return 0x399;
      case 0x1f3e:
        consumed = 1; return 0x399;
      case 0x1f3f:
        consumed = 1; return 0x399;
      case 0x1f40:
        consumed = 1; return 0x3bf;
      case 0x1f41:
        consumed = 1; return 0x3bf;
      case 0x1f42:
        consumed = 1; return 0x3bf;
      case 0x1f43:
        consumed = 1; return 0x3bf;
      case 0x1f44:
        consumed = 1; return 0x3bf;
      case 0x1f45:
        consumed = 1; return 0x3bf;
      case 0x1f48:
        consumed = 1; return 0x39f;
      case 0x1f49:
        consumed = 1; return 0x39f;
      case 0x1f4a:
        consumed = 1; return 0x39f;
      case 0x1f4b:
        consumed = 1; return 0x39f;
      case 0x1f4c:
        consumed = 1; return 0x39f;
      case 0x1f4d:
        consumed = 1; return 0x39f;
      case 0x1f50:
        consumed = 1; return 0x3c5;
      case 0x1f51:
        consumed = 1; return 0x3c5;
      case 0x1f52:
        consumed = 1; return 0x3c5;
      case 0x1f53:
        consumed = 1; return 0x3c5;
      case 0x1f54:
        consumed = 1; return 0x3c5;
      case 0x1f55:
        consumed = 1; return 0x3c5;
      case 0x1f56:
        consumed = 1; return 0x3c5;
      case 0x1f57:
        consumed = 1; return 0x3c5;
      case 0x1f59:
        consumed = 1; return 0x3a5;
      case 0x1f5b:
        consumed = 1; return 0x3a5;
      case 0x1f5d:
        consumed = 1; return 0x3a5;
      case 0x1f5f:
        consumed = 1; return 0x3a5;
      case 0x1f60:
        consumed = 1; return 0x3c9;
      case 0x1f61:
        consumed = 1; return 0x3c9;
      case 0x1f62:
        consumed = 1; return 0x3c9;
      case 0x1f63:
        consumed = 1; return 0x3c9;
      case 0x1f64:
        consumed = 1; return 0x3c9;
      case 0x1f65:
        consumed = 1; return 0x3c9;
      case 0x1f66:
        consumed = 1; return 0x3c9;
      case 0x1f67:
        consumed = 1; return 0x3c9;
      case 0x1f68:
        consumed = 1; return 0x3a9;
      case 0x1f69:
        consumed = 1; return 0x3a9;
      case 0x1f6a:
        consumed = 1; return 0x3a9;
      case 0x1f6b:
        consumed = 1; return 0x3a9;
      case 0x1f6c:
        consumed = 1; return 0x3a9;
      case 0x1f6d:
        consumed = 1; return 0x3a9;
      case 0x1f6e:
        consumed = 1; return 0x3a9;
      case 0x1f6f:
        consumed = 1; return 0x3a9;
      case 0x1f70:
        consumed = 1; return 0x3b1;
      case 0x1f71:
        consumed = 1; return 0x3b1;
      case 0x1f72:
        consumed = 1; return 0x3b5;
      case 0x1f73:
        consumed = 1; return 0x3b5;
      case 0x1f74:
        consumed = 1; return 0x3b7;
      case 0x1f75:
        consumed = 1; return 0x3b7;
      case 0x1f76:
        consumed = 1; return 0x3b9;
      case 0x1f77:
        consumed = 1; return 0x3b9;
      case 0x1f78:
        consumed = 1; return 0x3bf;
      case 0x1f79:
        consumed = 1; return 0x3bf;
      case 0x1f7a:
        consumed = 1; return 0x3c5;
      case 0x1f7b:
        consumed = 1; return 0x3c5;
      case 0x1f7c:
        consumed = 1; return 0x3c9;
      case 0x1f7d:
        consumed = 1; return 0x3c9;
      case 0x1f80:
        consumed = 1; return 0x3b1;
      case 0x1f81:
        consumed = 1; return 0x3b1;
      case 0x1f82:
        consumed = 1; return 0x3b1;
      case 0x1f83:
        consumed = 1; return 0x3b1;
      case 0x1f84:
        consumed = 1; return 0x3b1;
      case 0x1f85:
        consumed = 1; return 0x3b1;
      case 0x1f86:
        consumed = 1; return 0x3b1;
      case 0x1f87:
        consumed = 1; return 0x3b1;
      case 0x1f88:
        consumed = 1; return 0x391;
      case 0x1f89:
        consumed = 1; return 0x391;
      case 0x1f8a:
        consumed = 1; return 0x391;
      case 0x1f8b:
        consumed = 1; return 0x391;
      case 0x1f8c:
        consumed = 1; return 0x391;
      case 0x1f8d:
        consumed = 1; return 0x391;
      case 0x1f8e:
        consumed = 1; return 0x391;
      case 0x1f8f:
        consumed = 1; return 0x391;
      case 0x1f90:
        consumed = 1; return 0x3b7;
      case 0x1f91:
        consumed = 1; return 0x3b7;
      case 0x1f92:
        consumed = 1; return 0x3b7;
      case 0x1f93:
        consumed = 1; return 0x3b7;
      case 0x1f94:
        consumed = 1; return 0x3b7;
      case 0x1f95:
        consumed = 1; return 0x3b7;
      case 0x1f96:
        consumed = 1; return 0x3b7;
      case 0x1f97:
        consumed = 1; return 0x3b7;
      case 0x1f98:
        consumed = 1; return 0x397;
      case 0x1f99:
        consumed = 1; return 0x397;
      case 0x1f9a:
        consumed = 1; return 0x397;
      case 0x1f9b:
        consumed = 1; return 0x397;
      case 0x1f9c:
        consumed = 1; return 0x397;
      case 0x1f9d:
        consumed = 1; return 0x397;
      case 0x1f9e:
        consumed = 1; return 0x397;
      case 0x1f9f:
        consumed = 1; return 0x397;
      case 0x1fa0:
        consumed = 1; return 0x3c9;
      case 0x1fa1:
        consumed = 1; return 0x3c9;
      case 0x1fa2:
        consumed = 1; return 0x3c9;
      case 0x1fa3:
        consumed = 1; return 0x3c9;
      case 0x1fa4:
        consumed = 1; return 0x3c9;
      case 0x1fa5:
        consumed = 1; return 0x3c9;
      case 0x1fa6:
        consumed = 1; return 0x3c9;
      case 0x1fa7:
        consumed = 1; return 0x3c9;
      case 0x1fa8:
        consumed = 1; return 0x3a9;
      case 0x1fa9:
        consumed = 1; return 0x3a9;
      case 0x1faa:
        consumed = 1; return 0x3a9;
      case 0x1fab:
        consumed = 1; return 0x3a9;
      case 0x1fac:
        consumed = 1; return 0x3a9;
      case 0x1fad:
        consumed = 1; return 0x3a9;
      case 0x1fae:
        consumed = 1; return 0x3a9;
      case 0x1faf:
        consumed = 1; return 0x3a9;
      case 0x1fb0:
        consumed = 1; return 0x3b1;
      case 0x1fb1:
        consumed = 1; return 0x3b1;
      case 0x1fb2:
        consumed = 1; return 0x3b1;
      case 0x1fb3:
        consumed = 1; return 0x3b1;
      case 0x1fb4:
        consumed = 1; return 0x3b1;
      case 0x1fb6:
        consumed = 1; return 0x3b1;
      case 0x1fb7:
        consumed = 1; return 0x3b1;
      case 0x1fb8:
        consumed = 1; return 0x391;
      case 0x1fb9:
        consumed = 1; return 0x391;
      case 0x1fba:
        consumed = 1; return 0x391;
      case 0x1fbb:
        consumed = 1; return 0x391;
      case 0x1fbc:
        consumed = 1; return 0x391;
      case 0x1fc2:
        consumed = 1; return 0x3b7;
      case 0x1fc3:
        consumed = 1; return 0x3b7;
      case 0x1fc4:
        consumed = 1; return 0x3b7;
      case 0x1fc6:
        consumed = 1; return 0x3b7;
      case 0x1fc7:
        consumed = 1; return 0x3b7;
      case 0x1fc8:
        consumed = 1; return 0x395;
      case 0x1fc9:
        consumed = 1; return 0x395;
      case 0x1fca:
        consumed = 1; return 0x397;
      case 0x1fcb:
        consumed = 1; return 0x397;
      case 0x1fcc:
        consumed = 1; return 0x397;
      case 0x1fd0:
        consumed = 1; return 0x3b9;
      case 0x1fd1:
        consumed = 1; return 0x3b9;
      case 0x1fd2:
        consumed = 1; return 0x3b9;
      case 0x1fd3:
        consumed = 1; return 0x3b9;
      case 0x1fd6:
        consumed = 1; return 0x3b9;
      case 0x1fd7:
        consumed = 1; return 0x3b9;
      case 0x1fd8:
        consumed = 1; return 0x399;
      case 0x1fd9:
        consumed = 1; return 0x399;
      case 0x1fda:
        consumed = 1; return 0x399;
      case 0x1fdb:
        consumed = 1; return 0x399;
      case 0x1fe0:
        consumed = 1; return 0x3c5;
      case 0x1fe1:
        consumed = 1; return 0x3c5;
      case 0x1fe2:
        consumed = 1; return 0x3c5;
      case 0x1fe3:
        consumed = 1; return 0x3c5;
      case 0x1fe4:
        consumed = 1; return 0x3c1;
      case 0x1fe5:
        consumed = 1; return 0x3c1;
      case 0x1fe6:
        consumed = 1; return 0x3c5;
      case 0x1fe7:
        consumed = 1; return 0x3c5;
      case 0x1fe8:
        consumed = 1; return 0x3a5;
      case 0x1fe9:
        consumed = 1; return 0x3a5;
      case 0x1fea:
        consumed = 1; return 0x3a5;
      case 0x1feb:
        consumed = 1; return 0x3a5;
      case 0x1fec:
        consumed = 1; return 0x3a1;
      case 0x1ff2:
        consumed = 1; return 0x3c9;
      case 0x1ff3:
        consumed = 1; return 0x3c9;
      case 0x1ff4:
        consumed = 1; return 0x3c9;
      case 0x1ff6:
        consumed = 1; return 0x3c9;
      case 0x1ff7:
        consumed = 1; return 0x3c9;
      case 0x1ff8:
        consumed = 1; return 0x39f;
      case 0x1ff9:
        consumed = 1; return 0x39f;
      case 0x1ffa:
        consumed = 1; return 0x3a9;
      case 0x1ffb:
        consumed = 1; return 0x3a9;
      case 0x1ffc:
        consumed = 1; return 0x3a9;
      case 0xfb1d:
        consumed = 1; return 0x5d9;
      case 0xfb2a:
        consumed = 1; return 0x5e9;
      case 0xfb2b:
        consumed = 1; return 0x5e9;
      case 0xfb2c:
        consumed = 1; return 0x5e9;
      case 0xfb2d:
        consumed = 1; return 0x5e9;
      case 0xfb2e:
        consumed = 1; return 0x5d0;
      case 0xfb2f:
        consumed = 1; return 0x5d0;
      case 0xfb30:
        consumed = 1; return 0x5d0;
      case 0xfb31:
        consumed = 1; return 0x5d1;
      case 0xfb32:
        consumed = 1; return 0x5d2;
      case 0xfb33:
        consumed = 1; return 0x5d3;
      case 0xfb34:
        consumed = 1; return 0x5d4;
      case 0xfb35:
        consumed = 1; return 0x5d5;
      case 0xfb36:
        consumed = 1; return 0x5d6;
      case 0xfb38:
        consumed = 1; return 0x5d8;
      case 0xfb39:
        consumed = 1; return 0x5d9;
      case 0xfb3a:
        consumed = 1; return 0x5da;
      case 0xfb3b:
        consumed = 1; return 0x5db;
      case 0xfb3c:
        consumed = 1; return 0x5dc;
      case 0xfb3e:
        consumed = 1; return 0x5de;
      case 0xfb40:
        consumed = 1; return 0x5e0;
      case 0xfb41:
        consumed = 1; return 0x5e1;
      case 0xfb43:
        consumed = 1; return 0x5e3;
      case 0xfb44:
        consumed = 1; return 0x5e4;
      case 0xfb46:
        consumed = 1; return 0x5e6;
      case 0xfb47:
        consumed = 1; return 0x5e7;
      case 0xfb48:
        consumed = 1; return 0x5e8;
      case 0xfb49:
        consumed = 1; return 0x5e9;
      case 0xfb4a:
        consumed = 1; return 0x5ea;
      case 0xfb4b:
        consumed = 1; return 0x5d5;
      case 0xfb4c:
        consumed = 1; return 0x5d1;
      case 0xfb4d:
        consumed = 1; return 0x5db;
      case 0xfb4e:
        consumed = 1; return 0x5e4;
    }
  }
  if ( size )
  {
    consumed = 1; return *in;
  }
  consumed = 0; return 0;
}
