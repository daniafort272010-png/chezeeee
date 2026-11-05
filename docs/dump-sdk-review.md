# SDK review against the uploaded IL2CPP metadata

This is a static compatibility review of `dump.cs` and `il2cpp.h`, not a successful game-runtime test. The dump does not establish which installed game build produced it. Offsets below describe this dump only.

## Confirmed corrections

- `PlayerEyes.bodyRotation` and `headAngles`: use their compiler-generated backing-field names.
- `PlayerEyes.position` and `rotation`: use `get_position` / `get_rotation`; these are computed properties, not instance fields.
- `Projectile`: remove the nonexistent `hitInfo` accessor and its unused read in the update hook.
- `HitTest`: resolve `damageProperties` with its exact case.
- `PlayerWalkMovement`: use the bool field `grounded` and the `CurrentVelocity` method; `Ducking` is a float backing field on `BaseMovement`.
- `FIELD`: add field offsets in bytes, not in multiples of the C++ wrapper's size.

The SDK normally resolves these fields/methods by metadata name. Numeric offsets were not substituted blindly.

## Examples from this dump

| Type | Member | Offset / type |
| --- | --- | --- |
| BasePlayer | playerModel | 0x4E8 / PlayerModel reference |
| BasePlayer | eyes | 0x6F0 / PlayerEyes reference |
| BasePlayer | inventory | 0x6F8 / PlayerInventory reference |
| PlayerEyes | bodyRotation backing field | 0x44 / Quaternion |
| PlayerEyes | headAngles backing field | 0x54 / Vector3 |
| Projectile | initialVelocity | 0x18 / Vector3 |
| Projectile | drag | 0x24 / float |
| Projectile | gravityModifier | 0x28 / float |
| Projectile | thickness | 0x2C / float |
| Projectile | currentVelocity | 0x118 / Vector3 |
| Projectile | currentPosition | 0x124 / Vector3 |
| Projectile | traveledTime | 0x134 / float |

`eyes` and `inventory` are direct references in these files, not the GCHandle wrappers described in the earlier offset lists. `swimScale` and `swimSpeed` are both Vector3. The hard-coded PlayerProjectileUpdate layout (0x10/0x14/0x20/0x2C) agrees with this dump.

## Validation and remaining work

Ten isolated accessor checks passed on mock objects using the modified macros/classes. A negative control detected the original FIELD pointer-scaling bug. Seven metadata/source consistency checks passed. This is not a Windows solution build, DLL injection test, or end-to-end validation of Manipulator/Bullet TP.

Remaining review items include nested-class resolution (EntityRealm/Magazine), unchecked failed field/method lookups, stale unused HitInfo/visual accessors, and hook signature consistency. No complete module transplant from Vlone or guaranteed server acceptance is claimed. Do not treat this patch as a fully updated SDK.

The raw dumps and generated binaries were not added to the repository. Solution/project files were not changed.

## Input fingerprints (SHA-256)

- `dump.cs`: `57f6972e754b4ac614f54d53dd598326e315344f9351efc1de5ec4f324151914`
- `il2cpp.h`: `d13a488178ac1234e4997a297d72b1f39b298235279832ee559b4397f48c2f76`
