# Zero Comico / ScummVM - Stage 21

Stage 21 begins reconstructing the retail room/portal topology from
`room.isc`.

Branch: `scratch/zerocomico-stage21`.

## Proven serializer format

The original executable contains these room serializer strings:

```
Prefix: %s
backgrd: "%s"
objects: "%s"
portal: %s %s %s "%s" %s
map: %s
cameramap: %s
camera: %s
cameraspot: %s
```

The five-field portal arity is therefore treated as ground truth rather than
an inferred script convention.

## RoomTopologyParser

For each Room the parser records:

- Prefix;
- background;
- object set;
- navigation map;
- camera map;
- camera;
- camera spot;
- every five-field portal record.

Quoted portal field 4 stays a single value even if it contains whitespace or
`//`.

Malformed portal arity is rejected.

## Topology resolution without field-order guesses

The resolver does not assume that “field 1 is the shape” or “field 2 is the
target room”.

Instead every portal field is compared exactly against:

- declared Room names;
- Shape.shp entries explicitly typed `Portal`.

Unique matches become evidence. Multiple matches stay ambiguous.

That gives the runtime a safe path toward room changes:

```
Room
 -> portal raw five-field record
 -> exact room-name correlation
 -> exact Portal-shape correlation
 -> future proven field semantics
 -> room transition
```

## Validation

The focused test covers CRLF parsing, all proven room fields, exact portal
arity, quoted fields, unique and ambiguous room correlation, Portal-shape
matching and inline-comment handling.

The previous divergent Stage 21 branch is preserved as
`scratch/zerocomico-stage21-legacy`; the active branch is based on the
latest green Stage 20 line.
